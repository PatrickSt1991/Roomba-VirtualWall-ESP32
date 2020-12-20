# Roomba-VirtualWall-ESP32
ESP32 Virtuall Wall Roomba

Checking if Roomba is cleaning by checking the Domoticz Switch. IR Send for 60 minutes, cause Roomba if cleaning for about one hour.

Original: https://github.com/MKme/Roomba (Arduino nano)
<hr/>
<b>Handy DIY Roomba Virtual Wall with ESP32</b>

IR LED with it's positive leg connected via a current-limiting resistor(220 Ohm or similar) to the +3.3v, and it's negative leg to Pin5 (you can choose any other if you want, check esp32 pinout)

If you wish to measure the battery status:

<table>
  <tbody>
    <tr>
      <th>Pin On LED</th>
      <th>Pin on ESP-32</th>
    </tr>
    <tr>
      <td>LED -</td>
      <td>D5 Via resistor</td>
    </tr>
    <tr>
      <td>LED +</td>
      <td>3.3V</td>
    </tr>
  </tbody>
</table>

<table>
  <tbody>
    <tr>
      <td>Battery +</td>
      <td>27K Ohm (R1)</td>
    </tr>
    <tr>
      <td>Battery -</td>
      <td>100K Ohm (R2)</td>
    </tr>
    <tr>
      <td>R1 + R2</td>
      <td>D33</td>
    </tr>
  </tbody>
</table>
