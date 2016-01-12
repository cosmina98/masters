#!/bin/bash

# Tries to create instances similar to the one used on Section 5.2.3. 
# "Data sets with a postponed periodicity level" of the article
# "A hybrid algorithm for the unbounded knapsack problem"

# There's 200 instances per line on Table 1 "Data sets with a
# postponed periodicity", as they have the same parameters the
# seed needs to be different. We therefore need 200 distinct seeds.
# We generated this seeds with random.org.
seeds=(155213243 720843886 463795034 603562343 135458708 999292130 351997719 451986827 855531081 684448574
260379653 577782098 774727805 273864349 577520629 161344243 908468142 414892754 364656855 618866736
824350602 115726845 117967725 327529482 146571312 324200696 207893161 352626512 988774221 509888126
677916848 329274243 870038921 104835463 506110575 522080131 11639593 607801339 904998016 923188628
430006297 447231691 57171364 343141202 193169413 664964299 335197937 676454610 193021599 832538455
818489023 362775446 604095046 582917646 264578645 6971566 88272139 989083921 551593996 198519773
481113297 403017354 913132733 100579595 512463211 452751349 562370200 468933413 178912779 977952308
271358749 918482572 931292384 285045789 37069295 344310712 698894093 405400535 767427324 16986492
972099972 161938540 692706651 653138835 661561692 609893802 925211847 945306385 87557726 905865785
483963863 219788524 604520447 512984724 611662245 419785115 689366126 389788488 965728206 785559520
305421952 879272876 316472754 743151267 207123742 723015531 32492284 31387311 678444546 507400487
245781550 259905739 351421871 47721841 981619751 11371460 405621139 207178132 19360487 291809797
53290165 611080474 637657677 217583718 345851317 342662718 760619917 583556376 633218386 676563793
467683630 367303549 127422697 731218263 208479215 802259763 885510405 998577081 948885877 586663876
139860385 586185750 319663318 490853988 255129796 248156760 894606712 617392218 115731280 399496341
810086031 219535324 366491978 362367963 685047257 995770624 180237460 852514704 752268543 543190476
582600153 311293941 666650214 955524903 90959982 568312405 661392405 559438974 343653884 179141282
342094980 945974533 323499100 917541816 359051238 957468776 100860099 301097953 397831828 916911562
653992306 211003750 731025097 898011697 34442418 152642710 616010305 277361362 865461609 345268953
251424367 791104459 218633345 915027660 659711563 515702491 764445182 943590714 753852548 956890340)

# Follows 800 random generated capacities, by random.org. They are:
# 400 with values between 1020000 and 2*10^6; 400 with values between
# 1050000 and 2*10^6
declare -A cs
cs[1020000]="1596642 1669722 1768693 1357401 1496815 1450970 1967033 1874407 1974905 1154184
1491928 1871029 1448211 1072686 1195421 1467078 1466388 1568401 1284166 1596239
1949199 1929080 1767011 1363053 1960998 1523711 1419612 1649991 1646275 1390702
1943229 1553670 1058374 1509892 1331463 1544167 1239395 1461978 1639309 1333172
1797894 1278622 1138182 1692591 1637747 1745146 1911941 1366199 1342753 1291847
1301372 1191636 1055295 1745382 1110884 1290217 1966020 1181335 1872946 1384939
1685225 1708478 1330671 1250122 1174175 1175294 1257497 1034948 1720049 1341676
1474151 1590229 1494851 1918878 1994547 1360490 1360608 1913871 1230542 1508722
1431774 1309413 1094202 1583713 1660167 1805815 1466824 1375451 1864673 1896887
1523737 1868354 1319153 1580064 1457628 1594475 1830431 1657651 1363049 1139772
1385552 1649031 1228505 1121428 1116716 1439153 1230787 1742111 1220760 1533768
1180507 1734985 1221076 1790160 1457662 1659902 1649266 1732003 1745870 1601224
1825318 1471487 1056039 1142879 1186038 1086673 1864729 1780263 1777755 1421878
1851781 1245931 1298308 1138837 1963252 1507227 1639680 1647697 1154829 1192318
1389776 1382914 1224252 1574339 1195417 1391545 1593271 1755406 1072056 1308616
1733083 1172487 1703062 1466051 1313197 1536499 1368978 1782122 1302331 1149473
1334579 1107461 1152544 1494165 1115495 1192793 1405261 1859344 1610728 1806664
1337990 1858394 1522925 1533414 1136218 1846303 1605950 1511129 1358629 1503575
1092543 1736592 1635824 1527832 1257319 1432123 1479318 1622905 1593165 1286129
1416030 1448808 1479427 1920952 1813572 1831686 1735590 1758699 1554051 1333249
1302296 1817994 1376516 1262362 1411727 1905302 1411906 1701716 1465391 1544915
1619961 1816375 1128316 1640559 1757564 1048427 1790699 1072316 1288769 1081259
1863143 1267790 1099389 1818360 1427459 1239535 1106990 1027793 1804063 1970222
1284544 1102068 1590634 1137779 1126951 1074408 1278571 1966679 1668001 1155607
1763567 1951922 1383811 1842548 1601801 1087365 1441929 1172391 1031203 1329948
1188312 1057374 1307739 1120755 1793066 1397322 1781836 1973332 1341341 1042180
1529399 1323223 1809797 1488833 1276908 1803621 1633240 1802891 1812511 1574937
1470317 1155174 1982486 1834106 1111294 1599774 1222826 1581406 1786587 1977203
1534114 1648274 1125100 1034820 1118363 1577168 1892969 1090323 1558231 1150444
1528896 1618606 1559463 1600843 1962448 1627187 1159250 1551240 1175276 1292316
1047166 1412754 1553834 1174380 1768257 1979618 1823494 1849841 1152528 1888311
1679740 1892653 1456334 1608868 1449657 1605504 1401964 1776801 1026799 1767296
1262796 1689067 1234918 1439577 1945156 1724049 1456916 1492654 1369500 1579695
1469451 1951966 1834666 1046919 1650119 1938380 1564455 1500555 1796611 1257334
1033411 1932042 1882200 1794482 1997334 1718879 1722629 1422393 1686672 1053313
1546141 1935465 1077092 1211137 1595025 1634233 1637325 1815894 1404939 1453442
1464842 1052140 1698897 1557446 1094083 1927590 1116962 1505399 1601272 1295615
1124357 1241558 1558521 1507409 1348798 1932529 1025550 1221119 1338008 1456112
1961792 1454557 1051774 1461384 1512480 1287618 1473406 1443168 1233835 1022418
1889294 1372166 1612484 1358566 1076379 1879808 1340663 1588208 1600927 1988643"
cs[1050000]="1921419 1219357 1454414 1357468 1922678 1645290 1668209 1683740 1791319 1772748
1879272 1718268 1737310 1759284 1493710 1346441 1516079 1296479 1564883 1469822
1144033 1162419 1121559 1215934 1721931 1124061 1758833 1877833 1371468 1753551
1791253 1316858 1303950 1223551 1852067 1646295 1589912 1839898 1554542 1632432
1431009 1342027 1308116 1981234 1671826 1705072 1960694 1588106 1719691 1885270
1628893 1717059 1660715 1829290 1665196 1331979 1177060 1429622 1838167 1963585
1511246 1815855 1372451 1905262 1837476 1866491 1574025 1124507 1365922 1516191
1913342 1649912 1699090 1892360 1594027 1508521 1913587 1354156 1750101 1828891
1118941 1412688 1276856 1760249 1681311 1076370 1864548 1343923 1285467 1675850
1120598 1733550 1443632 1790143 1619800 1880526 1832326 1542674 1237842 1727794
1833682 1510009 1050804 1560616 1802639 1066735 1414745 1500717 1117457 1522008
1990034 1786644 1784152 1341145 1275575 1292625 1871751 1718032 1059295 1407069
1304509 1706034 1492341 1683897 1317467 1791900 1306781 1670601 1882045 1885467
1746191 1249067 1212998 1057822 1282861 1859860 1865617 1187478 1792337 1432971
1654159 1808596 1899788 1794567 1363463 1509156 1121210 1334410 1476738 1277769
1684696 1166941 1931956 1767800 1339353 1387312 1628661 1119161 1666901 1437885
1805082 1543686 1562554 1294794 1861478 1126439 1518352 1682902 1808168 1374017
1801899 1294842 1470900 1775018 1574253 1359135 1274846 1172820 1748679 1178635
1250851 1186420 1734427 1400387 1185129 1960781 1984529 1385020 1069655 1716265
1781640 1370617 1078405 1157104 1459660 1397428 1859917 1136886 1350534 1857737
1070927 1834312 1101005 1370625 1790901 1082699 1737764 1235104 1092800 1397683
1520334 1211849 1638836 1970782 1811012 1096797 1956620 1268196 1491886 1395411
1740612 1555680 1941864 1055607 1527612 1663465 1937199 1504585 1657653 1770066
1138464 1817277 1199985 1424773 1514867 1779899 1264149 1264854 1990074 1583118
1836320 1648147 1778708 1392767 1666059 1881901 1680650 1667472 1683953 1972129
1859030 1475582 1066549 1369464 1278054 1939219 1487240 1704577 1861688 1448324
1411262 1574848 1928098 1731060 1492155 1460704 1257276 1652347 1587922 1703551
1597428 1558514 1928871 1082461 1705264 1683052 1087619 1966876 1083167 1633454
1955629 1071297 1523440 1887143 1725449 1351938 1448382 1874745 1848306 1801895
1986038 1178643 1398376 1573094 1505907 1703623 1412984 1300582 1940571 1891422
1090580 1075219 1155251 1423973 1384075 1399641 1085001 1583866 1316905 1217026
1569117 1088107 1282410 1326555 1772861 1712788 1265615 1934170 1260793 1457462
1831682 1685870 1926700 1309370 1373891 1176150 1836582 1736605 1738971 1487829
1826003 1089092 1661008 1569635 1253064 1923117 1093219 1088514 1694873 1358828
1507701 1925039 1399313 1231133 1451827 1341378 1077587 1820735 1079942 1229097
1242628 1223585 1717385 1219946 1910506 1968439 1091135 1577913 1570154 1741570
1294224 1708024 1109425 1596348 1128617 1292526 1710624 1462198 1657107 1991813
1723343 1526716 1904117 1915753 1907077 1759906 1885004 1105643 1761639 1086171
1793626 1958298 1746932 1487691 1895016 1605504 1797255 1140199 1944509 1879060
1607708 1215868 1843191 1806147 1737566 1954823 1554487 1993775 1612443 1829250"

ns=(20000 50000 20000 50000)
wmins=(20000 20000 50000 50000)

declare -A c_counter
c_counter[200000]=0
c_counter[500000]=0

for line in 0 1 2 3; do
	n=${ns[$line]}
	wmin=${wmins[$line]}
	wmax="10$n"
	eval "cs2=(${cs[$wmax]})"
	for ((i=0; i < 200; ++i)); do
		c_ix=${c_counter[$wmax]}
		c=${cs2[$c_ix]}
		# The 500 below is a value fixed by the paper, not by the formula.
		# The formula use it and its quarter (125) as is in the paper.
		pyasukp -nosolve -form nsds2 -save "nsds2_n${n}wmin${wmin}-${i}-s${seeds[$i]}c${c}.ukp" -step 500 -n $n -wmin $wmin -wmax $wmax -seed ${seeds[$i]} -cap $c
		c_counter[$wmax]=`expr ${c_counter[$wmax]} + 1`
	done
done

