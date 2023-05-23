# figure_demo = [
#     IRule(
#         id = "R1",
#         threshold = 2,
#         exploit = Exploit(
#             pgn = 0,
#             da = 0
#         ),
#         interval = 9
#     ),
#     IRule(
#         id = "R5",
#         threshold = 1,
#         exploit = Exploit(
#             pgn = 0xEA00,
#             da = 0x0F,
#             sa=0x0B,
#             parameters = (
#                 [
#                     ParameterFilter(
#                         spn = 2540,
#                         value = [65269, 65259]
#                     )
#                 ]
#             )
#         ),
#         interval = 5
#   ),
#   CRule(
#      id = "R2",
#         threshold = 1,
#         exploit = Exploit(
#             pgn = 0,
#             da = 0,
#             sa = 0x31
#         ),
#   ),
#   CRule(
#      id = "R3",
#         threshold = 1,
#         exploit = Exploit(
#             pgn = 0,
#             da = 0,
#             sa = 0x0f,
#             parameters = [
#                 ParameterFilter(
#                     spn = 898,
#                     value = [50,100]
#                 )
#             ]
#         ),
#   ),
#   CRule(
#     id = "R4",
#     threshold = 1,
#     exploit = Exploit(
#             pgn = 0,
#             da = 0,
#             sa = 0x0f
#         ),
#     contextcarriers = [
#         ContextCarrier(
#             pgn = 0x0FEE1,
#             da = 0xff,
#             sa = 0x00,
#             parameters = [
#                 ParameterFilter(
#                     spn = 597,
#                     value = [1,1]
#                 )
#             ]
#         )
#     ]
#   )

# ]

from interface_structures import NetPFilter
from interface_structures import ParameterFilter
from interface_structures import MOI, Rule, CRule, IRule


experiment_rules = {
    IRule(
        id="Request Overload",
        threshold = 1,
        interval = 5,
        moi = MOI(
            pgn = 59904,
            da = 0
        )
    ),
    IRule(
        id = "Network Overload",
        threshold = 1,
        interval = 5,
        moi = MOI(
            pgn = 0,
            da = 0,
            sa=0
        )
    ),
    IRule(
        id = "Connection Exhaustion",
        threshold = 5,
        interval = 1250,
        moi = MOI(
            pgn = 60416,
            da = 0,
            sa=249
        )
    ),
    Rule(
        id = "EngineControlFromBodyController",
        threshold = 1,
        moi = MOI(
            pgn = 0,
            da = 0,
            sa = 0x21
        )
    ),
    Rule(
        id = "0%Torque",
        threshold = 1,
        moi = MOI(
            pgn = 0,
            da = 0,
            parameters = [
                ParameterFilter(
                    spn = 518,
                    value = (-125,-125)
                )
            ]        
            )
    ),
    CRule (
        id = "TSC1ABS",
        threshold = 1,
        moi = MOI(
            pgn = 0,
            da = 0,
            sa=11      
            ),
        context = [
            NetPFilter(
                pgn = 61441, da = 255, sa = 11,
                spn = 563, value = (0,0)    
            )
        ]
    ),
    CRule (
        id = "Claim",
        threshold = 1,
        moi = MOI(
            pgn = 0xEE00    
            ),
        context = [
            NetPFilter(
                pgn = 65265, da = 255, sa = 00,
                spn = 84, value = (5,300)
            )
        ]
    ),
    CRule (
        id = "RetarderJam",
        threshold = 1,
        moi = MOI(
            pgn = 0,
            da = 15  
            ),
        context = [
            NetPFilter(
                pgn = 65265, da = 255, sa = 0,
                spn = 84, value = (0,30)
            )
        ]
    )

}


test_demos = {
    Rule(
        id = "EngineControlFromBodyController",
        threshold = 1,
        moi = MOI(
            pgn = 0,
            da = 0,
            sa = 0x21
        )
    ),
    Rule(
        id = "0%Torque",
        threshold = 1,
        moi = MOI(
            pgn = 0,
            da = 0,
            parameters = [
                ParameterFilter(
                    spn = 518,
                    value = (0,0)
                )
            ]        
            )
    ),
    IRule (
        id = "IRule",
        threshold = 5,
        interval = 9,
        moi = MOI(
            pgn = 256,
            parameters = [
                ParameterFilter(
                    spn = 682,
                    value = (0,0)
                )
            ] 
        )
    ),
    CRule (
        id = "TSC1ABS",
        threshold = 1,
        moi = MOI(
            pgn = 0,
            da = 0,
            sa=11      
            ),
        context = [
            NetPFilter(
                pgn = 61441, da = 255, sa = 11,
                spn = 563, value = (0,0)
            )
        ]
    ),
        CRule (
        id = "NXTVIBE",
        threshold = 1,
        moi = MOI(
            pgn = 0,
            da = 0,
            sa=11      
            ),
        context = [
            NetPFilter(
                pgn = 61441, da = 255, sa = 11,
                spn = 563, value = (0,0)
            )
        ]
    ),

}