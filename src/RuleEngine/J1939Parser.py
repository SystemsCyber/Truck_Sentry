import math
# from os import path
from typing import Dict, List
import os


class MultipleSPNDefinitionsException(Exception):
    pass


class J1939Parser:
    def __init__(self, da_file=None):
        self.da_sqlite_file = os.path.dirname(os.path.realpath(__file__)) + '/DigiAnn.db'
        self.da_file = da_file
        if not os.path.isfile(self.da_sqlite_file): self._da_to_sqlite()
        import sqlite3
        self.da_sqlite_conn = sqlite3.connect(self.da_sqlite_file)
        self.da_sqlite_conn.row_factory = self._dict_factory
        self.da_sqlite_cursor = self.da_sqlite_conn.cursor()

    def __del__(self):
        self.da_sqlite_cursor.close()
        self.da_sqlite_cursor = None
        self.da_sqlite_conn.close()
        self.da_sqlite_conn = None

    @staticmethod
    def _form_parameter_info(SPNData) -> Dict:
        def length_byte_to_bit(val):
            if "." not in val:
                val = val + ".1"
            spl = val.split(".")
            # return (int(spl[0]) - 1) * 8 + int(spl[1]) # absolute bit position representation
            return [int(spl[_]) for _ in range(2)] # byte.bit representation

        # let us first get then length
        lsplit = SPNData["SPNLength"].split(" ")
        length = int(lsplit[0]) if "bit" in lsplit[1] else int(lsplit[0]) * 8
        # Next, let us get the position
        if "-" in SPNData["SPNPpositionInPGN"]:
            possplit = [x.strip() for x in SPNData["SPNPpositionInPGN"].split("-")]
        elif "," in SPNData["SPNPpositionInPGN"]:
            possplit = [x.strip() for x in SPNData["SPNPpositionInPGN"].split(",")]
        else:
            possplit = [SPNData["SPNPpositionInPGN"].strip(), SPNData["SPNPpositionInPGN"].strip()]
        possplit = [length_byte_to_bit(x) for x in possplit]
        # Finally lets parse the resolution and offset
        resolution = 0
        if SPNData["Resolution"] == "ASCII":
            resolution = 1
        elif SPNData["Resolution"] == "Binary" or SPNData["Resolution"][-10:] == "bit-mapped":
            resolution = 1
        else:
            try:
                # try:
                if SPNData["Resolution"].endswith("per bit") or SPNData["Resolution"].endswith("/bit"):
                    order = 0
                else:
                    order = int(SPNData["Resolution"].split("/")[1].split(" ")[0])
                res = SPNData["Resolution"].replace("/bit", "")
                resolution = eval(res.split(" ")[0]) / math.pow(2, order)
            except SyntaxError as e:
                if 'leading zeros in decimal integer literals are not permitted' in str(e):
                    resolution = float(resolution)

        offset = float(SPNData["Offset"].split(" ")[0].replace(",", ""))
        return {'pgn': int(SPNData["PGN"]), 'spn': int(SPNData["SPN"]), 'resolution': resolution, 'offset': offset, 'position': possplit,
                'length': length}

    def get_spn_info(self, spn, pgn=None):
        if pgn is None:
            row = self.da_sqlite_cursor.execute('SELECT * FROM DA WHERE SPN=?', (spn,)).fetchall()
            if len(row) > 1:
                raise MultipleSPNDefinitionsException("Multiple SPNs found, please provide a PGN")
            else:
                row = self._form_parameter_info(row[0])
        else:
            row = self._form_parameter_info(
                self.da_sqlite_cursor.execute('SELECT * FROM DA WHERE SPN=? and PGN=?',
                                              (spn, pgn)).fetchone())
        return row

    def get_pgn_info(self, pgn):
        row = self.da_sqlite_cursor.execute('SELECT SPN FROM DA WHERE PGN=?',(pgn,)).fetchall()
        return row

    def _da_to_sqlite(self):
        from sqlalchemy import create_engine
        import pandas

        da_SPNPGN = pandas.read_excel(self.da_file, sheet_name='SPNs & PGNs', header=3,
                                      usecols=['PGN', 'PARAMETER_GROUP_LABEL', 'ACRONYM', 'PG Description', 'EDP', 'DP',
                                               'PF', 'PS', 'Multipacket', 'Transmission Rate', 'PGN_Data_Length',
                                               'Default Priority', 'SPN_POSITION_IN_PGN', 'SPN', 'SPN_NAME',
                                               'SPN_DESCRIPTION', 'SPN_LENGTH', 'Resolution', 'Offset', 'Data Range',
                                               'Operational Range', 'Units', 'SP Document'])

        da_SPNPGN = da_SPNPGN.rename(
            columns={'PARAMETER_GROUP_LABEL': 'PGNLabel', 'Transmission Rate': 'TransmissionRate',
                     'PGN_Data_Length': 'PGNDataLength', 'Default Priority': 'DefaultPriority',
                     'SPN_POSITION_IN_PGN': 'SPNPpositionInPGN', 'SPN_NAME': 'SPNName',
                     'SPN_DESCRIPTION': 'SPNDescription', 'SPN_LENGTH': 'SPNLength',
                     'Data Range': 'DataRange', 'Operational Range': 'OperationalRange',
                     'SP Document': 'SPDocument'})

        da_SPNPGN = da_SPNPGN[da_SPNPGN['SPDocument'] == 'J1939DA']
        da_SPNPGN = da_SPNPGN[da_SPNPGN.PGN.notnull()]
        da_SPNPGN = da_SPNPGN[~da_SPNPGN['PGNDataLength'].str.contains('Variable', na=False)]
        da_SPNPGN = da_SPNPGN[da_SPNPGN['Multipacket'] == 'No']
        da_SPNPGN = da_SPNPGN[~(
                da_SPNPGN['Offset'].str.contains('Variant', na=False) | da_SPNPGN['Offset'].str.contains('Manufacturer',
                                                                                                         na=False))]
        #### !!! Interestingly, non-contigous SPNs are all filtered away upon using the above mentioned filters

        engine = create_engine('sqlite:///' + self.da_sqlite_file, echo=True)
        sqlite_connection = engine.connect()
        sqlite_table = "DA"
        da_SPNPGN.to_sql(sqlite_table, sqlite_connection, if_exists='fail')
        engine.dispose()

    def _dict_factory(self, cursor, row):
        d = {}
        for idx, col in enumerate(cursor.description):
            d[col[0]] = row[idx]
        return d

    def return_value_from_data(self, databytes: str, spn):
        """ Does not account for 10, 11, 12 bits i.e. ones which do not start and end at bytes boundaries but are multibyte
        return: is_valid_spn (not FF), raw value, processed value"""
        def decode_data(startpos, length, offset, resolution, dtype, data_byte_hex_string_array):
            DATA = ""
            prev_non_byte = False
            for byte in data_byte_hex_string_array:
                try:
                    int(byte,16)
                    byte = byte.rjust(2, '0')
                    if prev_non_byte:
                        print("Non hex data bytes found in DATA before padding")
                    DATA += byte
                except TypeError:
                    DATA += 'FF'
                    prev_non_byte = True

            length = min(len(DATA)*4, length)
            byte_length = math.ceil(length/8)
            bytes = [DATA[_*2:_*2+2] for _ in range(startpos[0] - 1, startpos[0] -1 + byte_length)][::-1]
            bts = "".join([bin(int(_,16))[2:].rjust(8,'0') for _ in bytes])
            try:
                bit_pos = startpos[1]
            except IndexError:
                bit_pos = 1
            bits = bts[::-1][bit_pos -1:bit_pos -1 + length][::-1]
            value = int(bits,2)*resolution + offset
            return int(bits,2) != math.pow(2,length) -1, [DATA[_*2:_*2+2] for _ in range(startpos[0] - 1, startpos[0] -1 + byte_length)][::-1], bits, value

        record = self.get_spn_info(spn, None)
        databytes = [databytes[_] + databytes[_+1] for _ in range(0,16,2)]
        return decode_data(record['position'][0],record['length'],record['offset'],record['resolution'], None, databytes)
    
    def check_valid_spns(self, databytes, pgn):
        valid_spns = []
        for spn in self.get_pgn_info(pgn):
            data = self.return_value_from_data(databytes, spn['SPN'])
            if (data[0]):
                valid_spns.append(spn['SPN'])
        return valid_spns
    

# da_file = '/home/subhojeet/.local/share/data/Mendeley Ltd./Mendeley Desktop/Downloaded/SAE International - Unknown - ' \
#           'J1939DA_202001.xlsx'
# jp = J1939Parser(da_file=None)
# print (jp.check_valid_spns("FFFFFFAABBBBBBBB", 61445))
# record = jp.get_spn_info(518, 0)
# print (record)

