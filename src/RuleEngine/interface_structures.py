from abc import ABC
from distutils.command.install_egg_info import safe_name
from enum import Enum
import math
from typing import  Tuple, List

# ============================================================================ #
# Config
# ============================================================================ #
j1939ParseLib = None


def set_j1939_parser(parser):
    global j1939ParseLib
    j1939ParseLib = parser
    

# ============================================================================ #
# Rule Interface
# ============================================================================ #
class ParameterFilter():
    def __init__(self, spn: int, value: Tuple[int,int]) -> None:
        """_summary_

        Args:
            spn (str): spn
            values (List of RANGE): A representation for the set of values
        """
        self.spn = spn
        self.value = value
        

class MOI():
    def __init__(self, pgn: int, da: int = None, sa: int = None, parameters: List[ParameterFilter] = []) -> None:
        """_summary_

        Args:
            id (str): WILDCARD
            parameters (List[Parameter], optional): List of parameters. Defaults to None.
        """        
        self.parameters = parameters
        self.sa = sa
        self.da = da
        self.pgn = pgn

class NetPFilter(ParameterFilter):
    def __init__(self, pgn: int, da:int, sa: int, spn: int, value: Tuple[int,int]) -> None:
        """_summary_

        Args:
            id (str): WILDCARD
            parameters (List[Parameter], optional): List of parameters. Defaults to None.
        """   
        self.sa = sa
        self.da = da
        self.pgn = pgn
        super().__init__(spn, value)

rindex = 0
class Rule(ABC):
    ARCH = 32
    def __init__(self, id: str, threshold: int, moi: MOI) -> None:
        """_summary_

        Args:
            name (str): _description_
            threshold (int): Must be less than what target system arch can support i.e. 2^ARCH -1
            interval (int): Must be less than 2^ARCH -1
            exploit (Exploit): _description_
            contextcarriers (List[ContextCarrier], optional): _description_. Defaults to [].
        """        
        global rindex
        self._index = rindex
        rindex += 1
        self.id = id
        self.threshold = threshold
        self.moi = moi
    
    def __str__(self) -> str:
        return "-".join([str(k[0]) for k in self.name.split(" ")])

    def __repr__(self) -> str:
        return self.id + ":" + str(self._index)

class IRule(Rule):
    def __init__(self, id: str, threshold: int, moi: MOI, interval: int):
        super().__init__(id, threshold, moi)
        self.interval = interval

class CRule(Rule):
    def __init__(self, id: str, threshold: int, moi: MOI, context: List[NetPFilter]):
        super().__init__(id, threshold, moi)
        self.context = context

class DefinitionException(Exception):
    pass