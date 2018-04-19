from skutterzero import Skutter

hettie = Skutter("D04")

print(hettie.LEDstartHueA)
hettie.LEDstartHueA = "#00FF00"
print(hettie.LEDstartHueA)
hettie.LEDstartHueA = 120
print(hettie.LEDstartHueA)
hettie.LEDstartHueA(120)
