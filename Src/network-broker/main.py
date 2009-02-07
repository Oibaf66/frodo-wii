import socket
import SocketServer
import struct

FRODO_NETWORK_MAGIC = 0x1976

HELLO = 99
LIST_PEERS = 98
PEER_CONNECT = 97
DISCONNECT = 96
PING = 95
ACK  = 94
STOP = 55

# Some of the Frodo network packets. There are more packets, but these
# are not interesting to the broker (and shouldn't be sent there either!)
packet_class_by_type = {
    HELLO        : HelloPacket,
    LIST_PEERS   : ListPeersPacket,
    PEER_CONNECT : ListPeersPacket,
    DISCONNECT   : DisconnectPacket,

    PING         : PingPacket,
    ACK          : PingPacket, # Ping and ack are the same

    STOP         : StopPacket,
}

class Packet:
    def __init__(self, data):
        """Create a new packet from raw data. Data should always be in network
byte order"""
        self.magic = struct.unpack(">H", data, offset = 0)[0]
        self.type = struct.unpack(">H", data, offset = 2)[0]
        self.size = struct.unpack(">L", data, offset = 4)[0]

    def __init__(self, magic, type, size):
        """Create a new packet"""
        self.magic = magic
        self.type = type
        self.size = size

    def get_magic(self):
        return self.magic

    def get_type(self):
        return self.type

    def get_size(self):
        return self.size

    def marshal(self):
        return struct.pack(">HHL", self.magic, self.type, self.size)

class PingPacket(Packet):
    def __init__(self, seq):
        Packet.__init__(self, FRODO_NETWORK_MAGIC, TYPE_PING, 9)
        self.seq = seq

    def get_seq(self):
        return self.seq

    def marshal(self):
        return Packet.marshal(self) + struct.pack(">B", self.seq)

class PeerInfo:
    def __init__(self, data):
        self.private_port = struct.unpack(">H", data, offset = 0)[0]
        self.public_port = struct.unpack(">H", data, offset = 2)[0]
        self.private_ip = self.mangle_ip(struct.unpack(">BBBB", data, offset = 4))
        self.public_ip = self.mangle_ip(struct.unpack(">BBBB", data, offset = 8))
        self.key = struct.unpack(">H", data, offset = 12)[0]
        self.is_master = struct.unpack(">H", data, offset = 16)[0]
        self.name = struct.unpack("32s", data, offset = 20)
        self.name[31] = 0

    def set_public_address(ip, port):
        self.public_ip = ip
        self.public_port = port

    def mangle_ip(self, ip):
        ip[0] = ~int(ip[0])
        ip[1] = ~int(ip[1])
        ip[2] = ~int(ip[2])
        ip[3] = ~int(ip[3])

    def marshal(self):
        priv_ip = self.mangle_ip(self.private_ip)
        pub_ip = self.mangle_ip(self.public_ip)
        return struct.pack(">HH4B4BHH32s",
                           self.private_port, self.public_port, priv_ip, pub_ip,
                           self.key, self.is_master, self.name)

class HelloPacket(Packet):
    def __init__(self, data):
        Packet.__init__(self, data)

        self.peer_info = PeerInfo(data[8:])

    def marshal(self):
        return Packet.marshal(self) + self.peer_info.marshal()

class Peer:
    def __init__(self, key, srv):
        self.hash_key = key
        self.srv = srv

    def get_hash_key(self):
        return self.hash_key

    def handle_packet(self, data):
        raw = Packet(data)
        pkg_cls = packet_class_by_type[raw.get_type()]
        pkg = pkg_cls(data)

        if pkg.type == ACK:
            pass
        elif pkg.type == HELLO:
            pass
        elif pkg.type == PEER_CONNECT:
            pass

class BrokerPacketHandler(SocketServer.DatagramRequestHandler):
    def handle(self):
        srv = self.server
        msg = self.rfile.read()

        peer = srv.get_peer(self.client_addr)
        try:
            reply = peer.handle_packet()
        except:
            # Sends crap, let's remove it
            srv.remove_peer(peer)

        if reply != None:
            self.wfile.write(reply)

class Broker(SocketServer.UDPServer):

    def __init__(self, host, req_handler):
        SocketServer.UDPServer.__init__(self, host, req_handler)
        # Instead of setsockopt( ... REUSEADDR ... )
        self.allow_reuse_address = True
        self.peers = {}

    def get_peer(self, key):
        "Return the peer for a certain key, or a new one if it doesn't exist"
        try:
            peer = self.peers[key]
        except KeyError, e:
            peer = Peer(key, self)
        return peer

    def remove_peer(self, peer):
        del self.peers[ peer.get_hash_key() ]

if __name__ == "__main__":
    print "Starting Frodo network broker"
    s = Broker( ("localhost", 46214), BrokerPacketHandler)
    s.serve_forever()
