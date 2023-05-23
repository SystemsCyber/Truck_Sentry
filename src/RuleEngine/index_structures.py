from typing import List, Set
from enum import Enum


class RLink():
    def __init__(self, rule_id, relation, indexes):
        self.rule_id = rule_id
        self.relation = relation
        self.indexes = indexes
    
    def __str__(self):
        return str(self.rule_id) + " " + str(self.relation) + "[" + ", ".join([str(k) for k in self.indexes]) + "]"

    def __serialize__(self):
        idstr = "\n" + " ".join([str(k) for k in self.indexes]) if len(self.indexes) > 0 else ""
        return str(self.rule_id) + " " + str(self.relation.value) + " "  + str(len(self.indexes)) + idstr

class Target():
    def __init__(self, np_l2rules, rlinks, last_byte) -> None:
        self.np_l2rules: Set[L2Rule] = np_l2rules
        self.rlinks: Set[RLink] = rlinks
        self.last_byte = last_byte

    def __str__(self):
        np_str = "np_rules: " + ", ".join([str(k) for k in self.np_l2rules])
        rlinks_str = "rlinks: \n" + "-"*20 + "\n" + "\n".join([str(r) for r in self.rlinks])
        return "last_byte: " + str(self.last_byte) + "\n" + np_str + "\n" + rlinks_str
    
    def __serialize__(self):
        np_l2rules_str = "\n" + " ".join([str(k) for k in self.np_l2rules]) if len(self.np_l2rules) > 0 else ""
        rlinks_str = "\n" + "\n".join([k.__serialize__() for k in self.rlinks]) if len(self.rlinks) > 0 else ""
        return str(self.last_byte) + "\n" + str(len(self.np_l2rules)) + np_l2rules_str + "\n" + str(len(self.rlinks)) + rlinks_str

class FieldFilter():
    def __init__(self, t_bytes, t_bits, t_masks, first_length, value):
        self.t_bytes = t_bytes
        self.t_bits = t_bits
        self.t_masks = t_masks
        self.first_length = first_length
        self.value = value

    def __str__(self):
        return "terminal_bytes: " + " ".join([str(k) for k in self.t_bytes]) + ", terminal_bits: " + " ".join([str(k) for k in self.t_bits]) + ", terminal_byte_masks: " + " ".join([str(k) for k in self.t_masks]) + ", first_byte_length: " + str(self.first_length) + ", value: " + " ".join([str(k) for k in self.value]) 
    
    def __serialize__(self):
        return " ".join([str(k) for k in self.t_bytes]) + " " + " ".join([str(k) for k in self.t_bits]) + " " + " ".join([str(int("".join(k),2)) for k in self.t_masks]) + " " + str(self.first_length) + " " + " ".join([str(k) for k in self.value]) 

# class SFieldFilter(FieldFilter):
#     def __init__(self, t_bytes, t_bits, t_masks, first_length, value):
#         self.prevm = False
#         super().__init__(t_bytes, t_bits, t_masks, first_length, value)
    
#     def __str__(self):
#         return super().__str__()

#     def __serialize__(self):
#         return super().__serialize__()


l2ruleindex = 0
class L2Rule():
    def __init__(self, threshold, interval, max_ncc, moi, context):
        global l2ruleindex
        self._index = l2ruleindex
        l2ruleindex += 1
        self.threshold = threshold
        self.interval = interval
        self.max_ncc = max_ncc
        self.moi: List[FieldFilter] = moi
        self.context: List[FieldFilter] = context # SFieldFilter is not needed
    
    def __serialize__(self):
        ccs_str = "\n" + "\n".join([cc.__serialize__() for cc in self.context]) if len(self.context) > 0 else ""
        moi_str = "\n" + "\n".join([ff.__serialize__() for ff in self.moi]) if len(self.moi) > 0 else ""
        return str(self._index) + " " + str(self.max_ncc) + " " + str(self.threshold) + " " + str(self.interval) + "\n" + str(len(self.moi)) + moi_str + "\n" + str(len(self.context)) + ccs_str


class RadixTrieNode():
    def __init__(self, value = 0 , length = 0, target:Target = None):
        self.value = value
        self.length = length
        self.target = target
        self._bval = ''

class RELTYPE(Enum):
    C = 2
    E = 1
    N = 0
    D = 3



