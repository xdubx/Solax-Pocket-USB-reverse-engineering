# What is this?

I try to revese the solar stick at the communication level with the solar inverter vom Solax.
In parallel, I'm looking at ghirda to maybe speed things up. I would be very grateful for any tips.


# Build of request USB


1. Request get info from inverter
```
0xaa ''
0x55 'U'
0x07 BEL
0x01 SOH Start of Heading
0x05 ENQ Enquiry
0x0c FF Form feed | seitenvorschub
0x01 SOH Start of Heading
```

2. Request
```
0xaa ''
0x55 'U'
'\x11'
'\x02'
'\x01'
'S'
'X'
'X'
'X'
'X'
'X'
'X'
'X'
'X'
'M'
'\x1f'
'\x04'
```

3. Requests get info from inverter

```
0xaa ''
0x55 'U'
0x07 BEL
0x01 SOH Start of Heading
0x05
0x0c
0x01 SOH Start of Heading
```

4. Requests request something

```
0xaa ''
0x55 'U'
0x07 BEL
0x01 SOH Start of Heading
0x16
0x1d
0x01 SOH Start of Heading
```

5. Requests request something and repeat maybe realTimeData ?
```
0xaa ''
0x55 'U'
0x07 BEL
0x01 SOH Start of Heading
0x0c
0x13
0x01 SOH Start of Heading
```



# Answer of inverter 
204 bytes

```
0xaa ''
0x55 'U'
'\xcf'  // constant
'\x01'  // constant 
'\x8c'  // constant

'\x19'      // changed
0x09 '\t'   // constant

0x0d '\r'   // changed 
'\x00'  

0x38 '8'    // changed
'\x01'      // changed

'\x8c'      // changed
'\x02'      // changed

'\x00'
'\x00'

0x32 '2'
'\x00'

'\x00'
'\x00'

0x47 'G'
'\x01' 

'\x00'
'\x00'

'\x88'
'\x13'

'\x02'  
'\x00'

0x25  '%' 
'\x02'  

'\x00'
'\x00'

'\x0e' 
'\x00'

'\x00'
'\x00'

'\x00'
'\x00'

'\x00'
'\x00'

'\x00'
'\x00'

'\x00'
'\x00'

'\x00'
'\x00'

'\x00'
'\x00'

'\x00'
'\x00'

'\x00'
'\x00'

'\x00'
'\x00'

'\x00'
'\x00'

'\x00'
'\x00'

'\x00'
'\x00'

'\x00'
'\x00'

'\x00'
'\x00'

'\x00'
'\x00'

'\x00'
'\x00'

'\x00'
'\x00'

'\x00'
'\x00'

'\x00'
'\x00'

'\x00'
'\x00'

'\x00'
'\x00'

'\x00'
'\x00'

'\x00'
'\x00'

'\x00'
'\x00'

0x24 '$'    
'\x00'  // 9216

'\x00'
'\x00'

'\x9a'  
'\x02'  // 39426

'\x00'
'\x00'

'\x00'
'\x00'

'\x00'
'\x00'

'\x00'
'\x00'

'\x00'
'\x00'

'\x00'
'\x00'

'\x00'
'\x00'

'\x00'
'\x00'

'\x00'
'\x00'

'\x00'
'\x00'

'\x00'
'\x00'

'\x00'
'\x00'

'\x00'
'\x00'

0x26  '&'    
'\x00'      // 9728

'\x00'
'\x00'

'\x00'
'\x00'

'\x00'
'\x00'

'\x00'
'\x00'

'\x00'
'\x00'

'\x00'
'\x00'

'\x00'
'\x00'

'\x00'
'\x00'

'\x00'
'\x00'

'\x00'
'\x00'

'\x00'
'\x00'

'\x00'
'\x00'

'\x00'
'\x00'

'\x00'
'\x00'

'\x00'
'\x00'

'\x00'
'\x00'

'\x00'
'\x00'

'\x00'
'\x00'

'\x00'
'\x00'

'\x00'
'\x00'

'\x00'
'\x00'

'\x00'
'\x00'

'\x00'
'\x00'

'\x00'
'\x00'

'\x00'
'\x00'

'\x00'
'\x00'

'\x00'
'\x00'

'\x00'
'\x00'

'\x00'
'\x00'

'\x00'
'\x00'

'\x00'
'\x00'

'\x00'
'\x00'

'\x00'
'\x00'

'\x00'
'\x00'

'\x00'
'\x00'

'\x00'
'\x00'

'\x00'
'\x00'

'\x00'
'\x00'

'\x00'
'\x00'

'\x00'
'\x00'

'\x00'
'\x00'

'\x00'
'\x00'

'\x00'
'\x00'

'\x00'
'\x00'

'\x83'  
'\x05'  // 33541
```




# Output on app
306 W Inverter
319 W Pannel
1,4kw Daly
54,9 kw sum of all time


58,1  total
0,6 daly
110 W Solar 
110 W Inverter 
100 W generator


## Data Structure
Solar
* pv1 voltage       66
* pv2 voltage       0
* pv1 current       1.6
* pv2 current       0
* pv1 input power   113
* pv2 input power   0

Grid 
* consume energy    0
* grid power        103W
* feed in energy    0

Off-grid
* off-grid Voltage  0
* off-grid current  0
* off-grid power    0
Battery 
* capacity          0
* voltage           0
* power             0
About
* Iverter sn
* dsp version       1.19
* arm version       1.18


# maybe struct of data 
*    temperature
*    energy_today
*    dc1_voltage
*    dc2_voltage
*    dc1_current
*   dc2_current
*   // AC Current
*    // AC Voltage
*    // AC Frequency
*    // AC Power
*    energy_total
*    runtime_total
*    status
*    errorCode