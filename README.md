# Roomba-VirtualWall-ESP32
ESP32 Virtuall Wall Roomba

Checking if Roomba is cleaning by checking the Domoticz Switch. IR Send for 60 minutes, cause Roomba is cleaning for about one hour.

Original: https://github.com/MKme/Roomba (Arduino nano)
<hr/>
<b>Handy DIY Roomba Virtual Wall with ESP32 and DS3231</b>

-- Never could get the ESP32 to sleep for the correct time, so I added and DS3231 wich sends a pulse to the ESP32 --

IR LED with it's positive leg connected via a current-limiting resistor(220 Ohm or similar) to the +3.3v, and it's negative leg to Pin5 (you can choose any other if you want, check esp32 pinout)

I'm using 2x 18650 Batteries giving a voltage of 8.28 which I bring down to 3.4ish with a step-down converter.

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
      <td>100K Ohm (R1)</td>
    </tr>
    <tr>
      <td>Battery -</td>
      <td>68K Ohm (R2)</td>
    </tr>
    <tr>
      <td>R1 + R2</td>
      <td>D15</td>
    </tr>
  </tbody>
</table>

<p align="center">
  <a target="_blank" rel="noopener noreferrer" href="https://github.com/PatrickSt1991/Roomba-VirtualWall-ESP32/blob/main/20210126_134120.jpg"><img src="https://github.com/PatrickSt1991/Roomba-VirtualWall-ESP32/raw/main/20210126_134120.jpg" width="700" style="max-width:100%;"></a>
</p>
