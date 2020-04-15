# Object-detect
![Image description](https://github.com/JamesCJH/Object-detect/blob/master/picture_in_Readme/find_center.png)  

## 所求：目標物中心點(width_M ,height_M)  
## 已知：圖形最左上點(width_S ,height_S)、圖形長&寬 height & width  
      
## Solution：  
1. 將輸入圖二值化-->BW_frame  

2. 由最左上點開始，由上而下，由左至右讀取像素值(黑=0，白=255)。 

3. 當接收到0-->1則是取得目標右邊邊界-->存入judge_left，反之則存入judge_right   

4. last_Diameter = judge_right - judge_left  
   temp_Diameter的值會逐漸變大，到達圓之直徑為最大值，之後逐漸變小，如下圖所示。
![Image description](https://github.com/JamesCJH/Object-detect/blob/master/picture_in_Readme/temp_diameter.png)  

![Image description](https://github.com/JamesCJH/Object-detect/blob/master/picture_in_Readme/coordinate.png)  


以相機拍到的履帶範圍為標的物，框出兩者在此之座標系  
![Image description](https://github.com/JamesCJH/Object-detect/blob/master/picture_in_Readme/coordinate%20conversion/coordinate%20conversion.png)  
  
Solution：二維座標的縮放+平移  
1.  
因為是要將影像座標系(pixel)-->控制器座標系(0.01 mm)  
![Image description](https://github.com/JamesCJH/Object-detect/blob/master/picture_in_Readme/coordinate%20conversion/formula_pixelTomm.png)
2.  
(**222**,0)-->(**0**,11550) 不考慮縮放下可得![Image description](https://github.com/JamesCJH/Object-detect/blob/master/picture_in_Readme/coordinate%20conversion/X%20scaling%20ratio%201.png)   
結合(1-2)和(1-3)得(1-4)得到  ![Image description](https://github.com/JamesCJH/Object-detect/blob/master/picture_in_Readme/coordinate%20conversion/X%20scaling%20ratio%202.png)  
3.  
(222,**0**)-->(0,**11550**) 由於Y為0，不好做計算，所以先由控制器推至相機，再做回推。  
(0,**11550**)--> (222,**0**)不考慮縮放下可得：![Image description](https://github.com/JamesCJH/Object-detect/blob/master/picture_in_Readme/coordinate%20conversion/Y%20scaling%20ratio%201.png)  
加入縮放比例(1-1)的倒數:![Image description](https://github.com/JamesCJH/Object-detect/blob/master/picture_in_Readme/coordinate%20conversion/Y%20scaling%20ratio%202.png)  
移項後：![Image description](https://github.com/JamesCJH/Object-detect/blob/master/picture_in_Readme/coordinate%20conversion/Y%20scaling%20ratio%203.png)  

將結果寫成一轉換矩陣：  ![Image description](https://github.com/JamesCJH/Object-detect/blob/master/picture_in_Readme/coordinate%20conversion/Transition%20matrix.png)

