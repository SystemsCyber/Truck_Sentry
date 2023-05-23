from typing import List
from index_trie import print_index
from index_structures import FieldFilter, L2Rule, Target, RELTYPE, RLink
from index_trie import form_index, serialize_index
from interface_structures import ParameterFilter, Rule, CRule, IRule
from j1939 import get_id_ternary, get_parameter_info
import math

temp_lkp_table = {}
l2rules = {} # parent rule, L2Rule

# # To find the partial byte boundary PGNs: select DISTINCT SPNPositioninPGN,SPNLength from SPNandPGN where Multipacket = 'No' and (SPNPositioninPGN like "%-%" or SPNPositioninPGN like "%,%") order by SPNLength;

keys = set()

def prepare_IDindex(part, rule ,relation, index):
    def update(key):
        id_indexed = temp_lkp_table.get(key,{})
        tup = id_indexed.get((rule.id,relation),(rule, relation,[]))
        if (index is not None):
            tup[2].append(index)
        id_indexed[(rule.id,relation)] = tup
        temp_lkp_table[key] = id_indexed
    
    update((part.pgn, part.da, part.sa))
    keys.add((part.pgn, part.da, part.sa))
    # update((part.pgn, part.da, None))
    # update((part.pgn, None, None))
    # pfilters = []
    # return pfilters

def process_parameterfilter(p: ParameterFilter):
    position, length, resolution, offset = get_parameter_info(p.spn)
    bytes = [position[0][0] -1, position[1][0] -1]
    bits = [position[0][1] -1, position[1][1] -1]
    mask1 = ["0" for _ in range(8)]
    mask2 = ["0" for _ in range(8)]
    if bytes[0] == bytes[1]:
        for bit in range(bits[0], bits[0] + length):
            mask1[7 - bit] = "1"
            mask2[7 - bit] = "1"
        first_length = length
    else:
        for bit in range(bits[0], 8):
            mask1[7 - bit] = "1"
        for bit in range(bits[1], bits[1] + length - (8*(bytes[1] - bytes[0] -1) + (7 - bits[0] + 1))):
            mask2[7 - bit] = "1"
        first_length = 7 - bits[0] + 1
    l2_val = (
    int((p.value[0] - offset) // resolution),
    int((p.value[1] - offset) // resolution)
    )
    return FieldFilter(bytes,bits,[mask1,mask2],first_length,l2_val)
    # pfilters.append(FieldFilter(bytes,bits,[mask1,mask2],first_length,l2_val))


from typing import List
from interface_structures import Rule


def pre_process(rules: List[Rule], arch):
    for rule in rules:        
        l2 = L2Rule(rule.threshold, rule.interval if isinstance(rule,IRule) else 0, len(rule.context) if isinstance(rule,CRule) else 0, moi = [], context = [])
        prepare_IDindex(rule.moi,rule,RELTYPE.E,None)
        l2.moi = [process_parameterfilter(p) for p in rule.moi.parameters]
        if isinstance(rule, CRule):
            for i in range(len(rule.context)):
                c = rule.context[i]
                prepare_IDindex(c,rule,RELTYPE.C,i)
                l2.context.append(process_parameterfilter(c))
        l2rules[rule] = l2

    # # Dump as temp lookup table
    # outf = open("truck_ips.txt", 'w')
    # outf.write(str(len(l2rules)))
    # for rule, l2rule in l2rules.items():
    #     # if len(l2rule.moi) > 0 or len(l2rule.context) > 0:
    #     print ("rule: ", rule.id, " l2rule: ", l2rule._index)
    #     outf.write("\n")
    #     outf.write(l2rule.__serialize__())
   
    # outf.write("\n")

    # for key in keys:
    #     print ("Key :", str(key))
    #     value = temp_lkp_table[key]
    #     outf.write("B\n")
    #     for triples in value.values():
    #         if isinstance(triples[0],CRule) or len(triples[0].moi.parameters) > 0:
    #             st = " " + " ".join([str(k) for k in triples[2]]) if len(triples[2]) > 0 else ""
    #             outf.write(str(l2rules[triples[0]]._index) + " " + 
    #                 str(triples[1].value) + " " + 
    #                 str(len(triples[2])) + st
    #             )
    #             outf.write("\n")

    # for key,value in temp_lkp_table.items():
    #     outf.write("B\n")
    #     for triples in value.values():
    #         if isinstance(triples[0],CRule) or len(triples[0].moi.parameters) > 0:
    #             st = " " + " ".join([str(k) for k in triples[2]]) if len(triples[2]) > 0 else ""
    #             outf.write(str(l2rules[triples[0]]._index) + " " + 
    #                 str(triples[1].value) + " " + 
    #                 str(len(triples[2])) + st
    #             )
    #             outf.write("\n")
    #     outf.write("B\n")

    # outf.close()

    arb_entries = {}
    for key,value in temp_lkp_table.items():
        rlinks = []
        np_l2rules = []
        last_byte = 0
        for triples in value.values():
            if not (isinstance(triples[0],CRule) or (len(triples[0].moi.parameters) > 0)):
                np_l2rules.append(l2rules[triples[0]]._index)
            else:
                rlinks.append(RLink(l2rules[triples[0]]._index,triples[1],triples[2]))
            
            if triples[1] == RELTYPE.E:
                for ff in l2rules[triples[0]].moi:
                    last_byte = max(last_byte, ff.t_bytes[1])
            else:
                for i in triples[2]:
                    last_byte = max(last_byte, l2rules[triples[0]].context[i].t_bytes[1])
        
        arb_entries[get_id_ternary(key[0],key[1],key[2])] = Target(np_l2rules,rlinks,last_byte)

    print ("\n".join([str(k) for k in arb_entries.keys()]))

    outf = open("truck_ips.txt", 'w')
    outf.write(str(len(l2rules)))
    for rule, l2rule in l2rules.items():
        print ("rule: ", rule.id, " l2rule: ", l2rule._index)
        outf.write("\n")
        outf.write(l2rule.__serialize__())
    outf.write("\n")
    index = form_index(arb_entries)
    ser = serialize_index(index)
    print_index(index,"index.pdf")
    outf.write(ser)
    outf.close()
    

# ARCH = 32

from demo_rulesets import test_demos, experiment_rules
pre_process(experiment_rules, Rule.ARCH)

# Gen random rule database'
# Teensy 3.6 heap maz = 244000
# Teendsy 4.1 heap max = 469000

# import random
# rule_type = random.randint(1,3)
# if rule_type == 1:
#     r = Rule()
# elif rule_type == 2:
#     r = IRule()
# else:
#     r = CRule()

# import os
# print (os.getcwd())
# import sqlite3
# conn = None
# try:
#     conn = sqlite3.connect("RuleEngine/DigiAnn.db")
# except Exception as e:
#     print(e)
# cur = conn.cursor()
# size = 20

# moi_pool = {}
# context_pool = {}
# cur.execute("SELECT distinct(PGN) FROM DA where Multipacket = 'No' and PS = 'DA'")
# for row in cur.fetchall():
#     moi_pool[row[0]] = row
#     if (len(moi_pool)) > size:
#         break

# cur.execute("SELECT distinct(PGN) FROM DA where Multipacket = 'No' and PS != 'DA'")
# for row in cur.fetchall():
#     context_pool[row[0]] = row
#     if (len(context_pool)) > size:
#         break



# if (random.random() > random.random()):
#     cur.execute("SELECT * FROM SPNandPGN where PGN = " + str(r.moi.pgn))
#     for row in cur.fetchall():
#         if (random.random() > random.random()):
#             pf = ParameterFilter(int(row[14]),)
#             r.moi.parameters.append()
#             r.moi.

# cur.execute("SELECT * FROM DA where PGN = 256")
# for row in cur.fetchall():
#     print (row[20])




