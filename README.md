1 這個程式是用來遠端控制汽車發動，主要方式是將汽車遙控器的啟動鍵焊出，再使用esp8266+繼電器遠程模擬按鍵。<br>
2 esp8266部分，使用MQTT的方式監聽指令，當收到對應指令時，觸發接腳閉合繼電器模擬按下遙控器，並發送LINE訊息。<br>
3 發送MQTT是使用home assistant。<br>

架構如下：
HA --> MQTT BROKER --> esp8266 --> 發送LINE訊息<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;└--->繼電器觸發汽車遙控器
