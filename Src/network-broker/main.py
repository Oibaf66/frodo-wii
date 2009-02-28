import socket
import SocketServer
import struct

FRODO_NETWORK_MAGIC = 0x1976

CONNECT_TO_BROKER  = 99 # Hello, broker
LIST_PEERS         = 98 # List of peers
CONNECT_TO_PEER    = 97 # A peer wants to connect
SELECT_PEER        = 93 # The client selects who to connect to
DISCONNECT         = 96 # Disconnect from a peer
PING               = 95 # (broker) are you alive?
ACK                = 94 # Answer to broker
STOP               = 55 # No more packets

class Packet:
    def __init__(self):
        """Create a new packet"""
        self.magic = FRODO_NETWORK_MAGIC
        self.type = 0
        self.size = 8

    def demarshal_from_data(self, data):
        """Create a new packet from raw data. Data should always be in network
byte order"""
        self.magic = struct.unpack_from(">H", data, offset = 0)[0]
        self.type = struct.unpack_from(">H", data, offset = 2)[0]
        self.size = struct.unpack_from(">L", data, offset = 4)[0]

    def get_magic(self):
        return self.magic

    def get_type(self):
        return self.type

    def get_size(self):
        return self.size

    def marshal(self):
        return struct.pack(">HHL", self.magic, self.type, self.size)

class StopPacket(Packet):
    def __init__(self):
        Packet.__init__(self)
        self.type = STOP

class SelectPeerPacket(Packet):
    def __init__(self):
        Packet.__init__(self)
        self.type = SELECT_PEER
        self.server_id = 0

    def demarshal_from_data(self, data):
        """Create a new packet from raw data."""
        Packet.demarshal_from_data(self, data)
        self.server_id = struct.unpack_from("<L", data, offset = 8)[0]

    def get_id(self):
        return self.server_id


class ConnectToBrokerPacket(Packet):

    def __init__(self):
        self.key = 0
        self._is_master = 0
        self.private_port = 0
        self.public_port = 0
        self.private_ip = ""
        self.public_ip = ""
        self.type = CONNECT_TO_BROKER
        self.name = ""
        self.serverid = 0

    def demarshal_from_data(self, data):
        Packet.demarshal_from_data(self, data)

        self.key = struct.unpack_from(">H", data, offset = 44)[0]
        self._is_master = struct.unpack_from(">H", data, offset = 46)[0]
        self.name = struct.unpack_from(">32s", data, offset = 48)[0]

    def get_key(self):
        return self.key

    def get_name(self):
        return self.name

    def is_master(self):
        return self._is_master

class ListPeersPacket(Packet):
    def __init__(self):
        Packet.__init__(self)
        self.n_peers = 0
        self.peers = []
        self.type = LIST_PEERS
        self.size = self.size + 24

    def add_peer(self, peer):
        self.peers.append(peer)
        self.n_peers = self.n_peers + 1
        self.size = self.size + 76

    def marshal(self):
        out = struct.pack(">L16sHxx", self.n_peers, "", 0)

        for peer in self.peers:
            out = out + struct.pack(">HH16s16sHH32sL",
                                    0, peer.public_port, "",
                                    peer.public_ip, peer.key,
                                    peer.is_master, peer.name,
                                    peer.id)

        return Packet.marshal(self) + out



class Peer:
    def __init__(self, addr, srv, id):
        self.srv = srv

        self.addr = addr
        self.public_ip, self.public_port = self.addr_to_ip_and_port(addr)
        # These will be set by the CONNECT_TO_BROKER packet below
        self.key = 0
        self.name = ""
        self.is_master = 0
        self.id = id

    def addr_to_ip_and_port(self, addr):
        ip = struct.unpack("@L", socket.inet_pton(socket.AF_INET, addr[0]))[0]
        port = addr[1]
        return "%08x" % (ip), port

    def handle_packet(self, pkt):
        if pkt.type == CONNECT_TO_BROKER:
            self.key = pkt.get_key()
            self.name = pkt.get_name()
            self.is_master = pkt.is_master()

            # Send list of peers if this is not a master
            if not self.is_master:
                lp = ListPeersPacket()

                for peer in self.srv.peers.itervalues():
                    if peer != self and peer.is_master:
                        lp.add_peer(peer)
                # And send the packet to this peer
                self.send_packet(lp.marshal())

        if pkt.type == SELECT_PEER:
            peer = self.srv.get_peer_by_id( pkt.get_id() )

            # Tell the peer that we have connected
            lp = ListPeersPacket()
            lp.add_peer(self)
            peer.send_packet( lp.marshal() )

            # These two are no longer needed
            self.srv.remove_peer(peer)
            self.srv.remove_peer(self)

    def send_packet(self, data):
        self.srv.socket.sendto(data + StopPacket().marshal(),
                               0, self.addr)

    def __str__(self):
        return '%s:%d "%s" %d %d' % (self.public_ip, self.public_port,
                             self.name, self.key, self.is_master)

class BrokerPacketHandler(SocketServer.DatagramRequestHandler):
    def get_packet_from_data(self, data):
        magic = struct.unpack_from(">H", data, offset = 0)[0]
        type = struct.unpack_from(">H", data, offset = 2)[0]

        if magic != FRODO_NETWORK_MAGIC:
            raise Exception("Packet magic does not match: %4x vs %4x\n" % (magic,
                                                                           FRODO_NETWORK_MAGIC) )
        try:
            out = packet_class_by_type[type]()
            out.demarshal_from_data(data)
            return out
        except KeyError, e:
            raise Exception("Unknown packet type %d" % (type))

    def handle(self):
        srv = self.server
        data = self.rfile.read()

        print "Got packet from", self.client_address
        try:
            pkt = self.get_packet_from_data(data)
        except Exception, e:
            print "Broken packet: ", e
            return

        peer = srv.get_peer(self.client_address)

        try:
            peer.handle_packet(pkt)
        except Exception, e:
            # Sends crap, let's remove it
            print "Handling packet failed, removing peer:", e
            srv.remove_peer(peer)

class Broker(SocketServer.UDPServer):

    def __init__(self, host, req_handler):
        SocketServer.UDPServer.__init__(self, host, req_handler)
        # Instead of setsockopt( ... REUSEADDR ... )
        self.allow_reuse_address = True
        self.peers = {}
        self.peers_by_id = {}
        self.id = 0

    def send_data(self, dst, data):
        self.socket.sendto(data, dst)

    def get_peer(self, key):
        "Return the peer for a certain key, or a new one if it doesn't exist"
        try:
            peer = self.peers[key]
        except KeyError, e:
            peer = Peer(key, self, self.id)
            self.peers[key] = peer
            self.peers_by_id[self.id] = peer
            self.id = self.id + 1
        return peer

    def get_peer_by_id(self, id):
        return self.peers_by_id[id]

    def get_peer_by_name_key(self, name, key):
        for k,v in self.peers.iteritems():
            if name == v.get_name() and key == v.get_key():
                return v
        return None

    def remove_peer(self, peer):
        del self.peers[ peer.addr ]
        del self.peers_by_id[ peer.id ]

# Some of the Frodo network packets. There are more packets, but these
# are not interesting to the broker (and shouldn't be sent there either!)
packet_class_by_type = {
    CONNECT_TO_BROKER : ConnectToBrokerPacket,
    SELECT_PEER : SelectPeerPacket,
}

if __name__ == "__main__":
    print "Starting Frodo network broker"
    s = Broker( ("localhost", 46214), BrokerPacketHandler)
    s.serve_forever()
