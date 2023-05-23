from J1939Parser import J1939Parser

da_file = '/home/subhojeet/.local/share/data/Mendeley Ltd./Mendeley Desktop/Downloaded/SAE International - Unknown - J1939DA_202001.xlsx'
parser = J1939Parser(da_file)

def get_id_ternary(pgn, da, sa) -> str:
    # id_vec = "0" # first bit is SOF
    # id_vec += "*"*3 # priority
    id_vec = "0" #EDP
    id_vec += bin((pgn & 0x1ffff) >> 16)[2:]  # DP
    pf = (pgn & 0x0ff00) >> 8
    id_vec += bin(pf)[2:].rjust(8, '0')[:6]
    id_vec += "11"  # SRR, IDE
    id_vec += bin(pf)[2:].rjust(8, '0')[6:]
    if pf >= 240:
        id_vec += bin(pgn & 0x000ff)[2:].rjust(8, '0')
    else:
        id_vec += '*' * 8 if da is None else bin(da)[2:].rjust(8, '0')
    id_vec += '*' * 8 if sa is None else bin(sa)[2:].rjust(8, '0')
    return id_vec

def get_parameter_info(spn):
    _record = parser.get_spn_info(spn)
    return _record['position'], _record['length'], _record['resolution'], _record['offset']


    