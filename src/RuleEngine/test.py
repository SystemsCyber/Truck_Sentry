from unittest.util import safe_repr
from j1939 import get_parameter_info
# from core import pre_process
# from interface_structures import ContextCarrier, Exploit, Parameter, Rule


# figure_demo = [
#     Rule (
#         "Demo1",
#         Exploit(0x0,0x0,0x0b),
#         [
#             ContextCarrier(0xfef1,0xff,0x0, [
#                 Parameter(84,[40,150]),
#                 Parameter(599,[1,1])
#             ])
#         ],
#         threshold=10,
#         interval=50
#     ),
#      Rule (
#         "Demo2",
#         Exploit(0x0,0x0, None, [
#             Parameter(695,[1,1]),
#             Parameter(898,[3000,7000])
#         ]),
#         [
#             ContextCarrier(0xfef1,0xff,0x0, [
#                 Parameter(84,[30,50]),
#                 Parameter(595,[1,1])
#             ])
#         ],
#         threshold=0,
#         interval=100
#     ),
#     Rule(
#         "Demo3",
#         Exploit(0x0),
#         threshold=1,
#         interval=10
#     ),
#     Rule(
#         "Demo4",
#         Exploit(0xea00),
#         threshold=0,
#         interval=0
#     ),
#     Rule(
#         "Demo5",
#         Exploit(0xea00, 0x0),
#         threshold=1,
#         interval=10
#     )
# ]

# pre_process(figure_demo, Rule.ARCH)


position, length, resolution, offset = get_parameter_info(4664)
bytes = [position[0][0] -1, position[1][0] -1]
bits = [position[0][1] -1, position[1][1] -1]
mask1 = ["0" for _ in range(8)]
mask2 = ["0" for _ in range(8)]
first_length = 0
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
print (bytes, " :: " , bits, " :: " , "".join([k for k in mask1]), " :: " , "".join([k for k in mask2]), " :: ", first_length)