Regression Testing
==================

 1. test01: Create a tapN device
 2. test02: Create a tunN device
 3. test03: Create a tap0 device
 4. test04: Create a tun0 device
 5. test05: Create a tapN device and set its mtu to 1400
 6. test06: Create a tunN device and set its mtu to 1400
 7. test07: Create a tapN device and set its MAC address to 54:1a:13:ef:b6:b5
 8. test08: Create a tunN device and set its MAC address to 54:1a:13:ef:b6:b5
 9. test09: Create a tapN device and set its MAC address to random
10. test10: Create a tunN device and set its MAC address to random
11. test11: Create a tapN device and turn it up
12. test12: Create a tunN device and turn it up
13. test13: Create a tapN device, turn it up and set its IPv4 address
14. test14: Create a tunN device, turn it up and set its IPv4 address
15. test15: Create a tapN device, turn it up and set its IPv6 address
16. test16: Create a tunN device, turn it up and set its IPv6 address
17. test17: Create a tapN device, set its IPv4 address and turn it up
18. test18: Create a tunN device, set its IPv4 address and turn it up
19. test19: Create a tapN device, set its IPv6 address and turn it up
20. test20: Create a tunN device, set its IPv6 address and turn it up
21. test21: Create a tapN device and set its IP address to NULL
22. test22: Create a tunN device and set its IP address to NULL
23. test23: Create a tapN device and set its netmask to a negative value
24. test24: Create a tunN device and set its netmask to a negative value
25. test25: Create a tapN device and set its IP address to a dumb value
26. test26: Create a tunN device and set its IP address to a dumb value
27. TODO ~~test27: Create a tapN device and set its netmask to a dumb value~~
28. TODO ~~test28: Create a tunN device and set its netmask to a dumb value~~
29. test29: Double create a tapN device
30. test30: Create a device with a wrong type
31. test31: Create a device with a wrong number
32. test32: Create a tapN device and check its pre-existing MAC address
33. test33: Create a tap0 persistent device and destroy it
34. test34: Create a tun0 persistent device and destroy it
35. test35: Create a tap0 persistent device and release it
36. test36: Create a tun1 persistent device and release it
37. test37: Set a log callback and generate an error
38. RETIRED ~~test38: Set a log callback and generate all possible error levels~~
39. test39: Enable debug for a tap device
40. test40: Enable debug for a tun device
41. test41: Set a new name to an interface
42. test42: Set a NULL name to an interface
43. test43: Set a name of more than IF_NAMESIZE to an interface
44. test44: Set a description to an interface
45. test45: Set a NULL description to an interface
46. test46: Set a description to an interface and check it

