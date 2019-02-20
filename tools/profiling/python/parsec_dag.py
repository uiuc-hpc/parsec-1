# parsec_dag.py
# Python module to represent a DAG of tasks for PaRSEC profiling

import networkx as nx
from collections import namedtuple
import re
import sys

class ParsecDAG:
    """A DAG of PaRSEC tasks"""

    def __init__(self):
        self.dag = nx.DiGraph()
        self.idtoname = dict()
        self.nametoid = dict()
        self.ParsecTaskID = namedtuple("ParsecTaskID", ["tpid", "did", "tid"])

    def _load_parsec_dot_file(self, f):
        """Module-private function. Adds nodes and links found in a file to the DAG being built

        Parameters
        ----------
        f: str
           A file name (DOT file, generated by PaRSEC with PARSEC_PROF_GRAPHER)
        dag: networkx.DiGraph
           The DAG being built

        Returns
        -------
        Nothing, but add nodes and links to the DAG
        
        Raises
        ------
        All execeptions due to file errors
        Additional exceptions when the file does not follow the format that PaRSEC is supposed to produce
        """
        node = re.compile(r'([^ ]+)..shape="polygon",style=filled,fillcolor="#([^"]+)",fontcolor="black",label=".([0-9]+).([0-9]+). ([^\(]+)\(([0-9, ]+)\).([0-9,\[\] ]+).<([0-9]+)>{([0-9]+)}".tooltip="tpid=([0-9]+):did=([0-9]+).tname=([^:]+):tid=([0-9]+)')
        link = re.compile('([^ ]+) -> ([^ ]+) .label="([^=]+)=>([^,]+)",color="#([^"]+)",style="([^"]+)"')
        start = re.compile('digraph G {')
        end   = re.compile('}') 
        nb = 1
        with open(f) as fp:
            line = fp.readline()
            while line:
                res = node.match(line)
                if( res ):
                    if( len(res.groups()) != 13 ):
                        raise Exception('Node lines are expected to provide 13 arguments, %d found in `%s` (line %d of %s)' % (len(res.groups()), line, nb, f) )
                    name = res.group(1)
                    parsec_id = self.ParsecTaskID(tpid = int(res.group(9)), tid = int(res.group(13)), did = int(res.group(11)))
                    self.idtoname[parsec_id] = name
                    self.nametoid[name] = parsec_id
                    self.dag.add_node(name, fc = res.group(2), thid  = int(res.group(3)), vpid = int(res.group(4)), label = res.group(5),
                                param = res.group(6), local = res.group(7), prio  = int(res.group(8)),
                                did = int(res.group(11)), tid = int(res.group(13)), tpid = int(res.group(9)))
                else:
                    res = link.match(line)
                    if( res ):
                        if( len(res.groups()) != 6 ):
                            raise Exception('Link lines are expected to provide 6 arguments, %d found in `%s` (line %d of %s)' % (len(res.groups()), line, nb, f) )
                        src      = res.group(1)
                        dst      = res.group(2)
                        self.dag.add_edge(src, dst, flow_src = res.group(3), flow_dst = res.group(4), color = res.group(5), style = res.group(6))
                    else:
                        res = start.match(line)
                        if( not res ):
                            res = end.match(line)
                            if( not res ):
                                raise Exception('Line `%s` does not match node or link (line %d of %s)' % (line, nb, f))
                line = fp.readline()
                nb += 1

    def load_parsec_dot_files(self, files):
        """Builds a NetworkX DiGraph from a set of DOT files generated by PaRSEC Prof Grapher
        
        Parameters
        ----------
        files: list of str
          The files to load
        
        Returns
        -------
        Nothing, but loads the files into the internal dag
        
        dag: networkx.DiGraph
          a Directed Graph holding all the nodes (tasks) and edges (flows) found in the dot files
          Each node is decorated with thid (thread id), vpid (virtual process id), label (task class name),
            param (parameters), local (values of local variables), prio (priority of the task),
            did (dictionary entry. Corresponds to the 'type' of events in the HDF5), tid (Task Identifier,
            corresponds to the 'id' of events in the HDF5, and tpid (Taskpool Identifier, corresponds to
            the 'taskpool_id' of events in the HDF5).
          Each edge is decorated with flow_src (the name of the source flow), flow_dst (the name of the
            destination flow), color (a suggested color for the link), and style (a suggested style for
            the link).
        """
        for f in files:
            self._load_parsec_dot_file(f)

    def node_from_name(self, name):
        """Returns a node from its internal name"""
        return self.dag.nodes[name]

    def node_from_id(self, tpid, did, tid):
        """Returns a node from its identifiers"""
        parsec_id = self.ParsecTaskID(tpid = int(tpid), did = int(did), tid = int(tid))
        return self.dag.nodes[ self.idtoname[parsec_id] ]

    def successors_from_name(self, name):
        """Returns the list of nodes successors of name"""
        return self.dag[name]

    def successors_from_id(self, tpid, did, tid):
        """Returns the list of nodes successors of (tpid, did, tid)"""
        parsec_id = self.ParsecTaskID(tpid = int(tpid), did = int(did), tid = int(tid))
        return self.dag[ self.idtoname[parsec_id] ]

if __name__ == '__main__':
    print "Loading all DOT files: '%s'" % (",".join(sys.argv[1:]))
    dag = ParsecDAG()
    dag.load_parsec_dot_files(sys.argv[1:])
    print type(dag)
    print "DAG has %d nodes and %d edges" % (nx.number_of_nodes(dag.dag), nx.number_of_edges(dag.dag))
