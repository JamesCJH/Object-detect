# Object-detect
 OpenCV4.1.1  
![Image description](https://github.com/JamesCJH/Object-detect/blob/master/picture_in_Readme/find_center.png)  

所求：目標物中心點(width_M ,height_M)  
已知：圖形最左上點(width_S ,height_S)、圖形長&寬 height & width  
      
Solution：  
1. 將輸入圖二值化-->BW_frame  

2. 由最左上點開始，由上而下，由左至右讀取像素值(黑=0，白=255)。 

3. 當接收到0-->1則是取得目標右邊邊界-->存入judge_left，反之則存入judge_right   

4. last_Diameter = judge_right - judge_left  
   temp_Diameter的值會逐漸變大，到達圓之直徑為最大值，之後逐漸變小，如下圖所示。
![Image description](https://github.com/JamesCJH/Object-detect/blob/master/picture_in_Readme/temp_diameter.png)  

![Image description](https://github.com/JamesCJH/Object-detect/blob/master/picture_in_Readme/coordinate.png)
