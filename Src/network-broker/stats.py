import sys, pickle
from operator import itemgetter

class Container:
    def __init__(self):
        self.country_count = {}
        self.last_10 = []

    def add_country(self, country):
        try:
            cur = self.country_count[country]
        except KeyError, e:
            cur = 0

        self.country_count[country] = cur + 1

    def add_connection(self, who, country):
        s = "%s (%s)" % (who, country)
        self.last_10 = [s] + self.last_10[:9]
        self.add_country(country)

class HtmlGenerator:
    def __init__(self, container):
        self.container = container

    def generate(self, outf):
        sorted_countries = sorted(self.container.country_count.items(),
                                  reverse=True, key=itemgetter(1))

        outf.write("<html><body>\n")
        outf.write("<H3>Last %d connections</H3>\n" % (len(self.container.last_10)) )
        for item in self.container.last_10:
            outf.write("%s<br>\n" % (item) )

        outf.write("<H3>Country list</H3>\n")
        count = 1
        for country, num in sorted_countries:
            outf.write("<b>%3d</b>. %s (%d)<br>\n" % (count, country, num) )
            count = count + 1
        outf.write("</body></html>\n")


g_stat = None
def save(filename):
    global g_stat

    of = open(filename, "wb")
    pickle.dump(g_stat, of)
    of.close()

def generate_html(filename):
    of = open(filename, "wb")
    hg = HtmlGenerator(g_stat)
    hg.generate(of)
    of.close()


def load(filename):
    global g_stat

    try:
        of = open(filename, "r")
        g_stat = pickle.load(of)
        of.close()
    except:
        g_stat = Container()

def add_connection(who, country):
    g_stat.add_connection(who, country)

if __name__ == "__main__":
    load("/tmp/vobb")
    for i in range(0, 10):
        add_connection("MABOO", "Unknown country")
    add_connection("SIMONK", "Sweden")
    add_connection("SIMONK", "Sweden")
    add_connection("SIMONK", "Sweden")
    add_connection("Linda", "Germany")
    add_connection("Linda", "Germany")
    save("/tmp/vobb")

    hg = HtmlGenerator(g_stat)
    hg.generate(sys.stdout)
