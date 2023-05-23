from collections import deque
import graphviz

id = 0
class Node():
    def __init__(self, length, value, target, _hind, _bval) -> None:
        global id
        self._id = id
        id += 1
        self.value = value
        self.length = length
        self.target = target
        self._hind = _hind
        self._bval = _bval
        self.zchild = None
        self.ochild = None
    
    def __str__(self):
        val = str(self.value) if self.length > 0 else ''
        _bval = str(self._bval) if self.length > 0 else ''
        return "value: " +  val + "\n" + \
                "bval: " + _bval + "\n" + \
                "length: " + str(self.length)

    def __repr__(self):
        trg = '0' if self.target is None else "1\n" + self.target.__serialize__() 
        return str(self.length) + " " + str(self.value) + " " + trg

def form_index(entries):
    keys = []
    indexes = []
    for k, v in entries.items():
        keys.append(k)
        indexes.append(v)

    root = Node(0,0,set(range(len(keys))), 0, '')
    q = deque()
    q.append(root)
    while len(q) > 0:
        node: Node = q.popleft()
        bit_dict = {}
        for _ in node.target:
            bv = keys[_]
            try:
                bit = bv[node._hind]
                vinds = bit_dict.get(bit, set())
                vinds.add(_)
                bit_dict[bit] = vinds
            except IndexError:
                node.target = indexes[_]
                break # If no bit is found, then we are at the end of a prefix and there are no duplicate prefixes, hence we are done i.e. there exists not more targets; this also means that bit_dict will be of 0 length
        if len(bit_dict) == 0: continue

        if len(bit_dict) == 1:
            bit = list(bit_dict.keys())[0]
            if (bit == '*'):
                node.target = indexes[_]
            else:
                node.value = node.value << 1 | int(bit,2)
                node._hind += 1
                node._bval += bit
                node.length += 1
                q.append(node)
        else:
            if ('*' in bit_dict.keys()):
                node.target = indexes[list(bit_dict['*'])[0]] # Again there could only be 1 uniq item with this prefix and with a '*' following it
            else:
                node.target = None

            for bit, v in bit_dict.items():
                if (bit == '0'):
                    node.zchild = Node(
                    length=1,
                    value=int(bit,2),
                    target=set(),
                    _bval=str(bit),
                    _hind = node._hind + 1)
                    node.zchild.target.update(v)
                    q.append(node.zchild)
                    
                if (bit == '1'):
                    node.ochild = Node(
                    length=1,
                    value=int(bit,2),
                    target=set(),
                    _bval=str(bit),
                    _hind = node._hind + 1)
                    node.ochild.target.update(v)
                    q.append(node.ochild)
    return root

def print_index(root, filename):
    graph = graphviz.Digraph(graph_attr={'rankdir': 'LR'})
    
    def pr_node(node):
        if node is None: return
        graph.node(str(node._id) + "-n",label=str(node))
        if node.target is not None:
            graph.node(str(node._id) + "-t",
                       label=str(node.target), shape='note')
            graph.edge(str(node._id) + "-n", str(node._id) + "-t", "target")

        if (node.zchild is not None):
            graph.edge(str(node._id) + "-n", str(node.zchild._id) + "-n", "0")
            pr_node(node.zchild)
        if (node.ochild is not None):
            graph.edge(str(node._id) + "-n", str(node.ochild._id) + "-n", "1")
            pr_node(node.ochild)
    
    pr_node(root)
    graph.render(outfile=filename,format='pdf', cleanup=True, view=False)
    return None


def serialize_index(root):
    # inoreder traversal and print node
    ser = ""
    def serialize_node(node):
        nonlocal ser
        if node is None: 
            ser += "-1"
            return
        ser += node.__repr__()
        ser += "\n"
        serialize_node(node.zchild)
        ser += "\n"
        serialize_node(node.ochild)
    serialize_node(root)
    return ser


# root = form_index({
#     '1111': "c..",
#     '10**': "a..",
#     '0110': "b..",
#     '0100': "a.."
#     })
# print_index(root, 'IDFieldTrie.pdf')