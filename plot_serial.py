import serial
import argparse
import time
import matplotlib.pyplot as plt
import matplotlib.animation as animation

# Set up argument parser
parser = argparse.ArgumentParser(description='Plot serial data in real-time.')
parser.add_argument('--device', type=str, required=False, help='Serial device (e.g., /dev/ttyUSB0)', default='/dev/ttyUSB0')
parser.add_argument('--baudrate', type=int, required=False, help='Baudrate (e.g., 115200)', default=115200)
args = parser.parse_args()

# Configure the serial port
ser = serial.Serial(args.device, args.baudrate)



class AnimationPlot:
    def animate(self, i, dataList, ser):
        arduinoData_string = ser.readline().decode('ascii') # Decode receive Arduino data as a formatted string
        #print(i)                                           # 'i' is a incrementing variable based upon frames = x argument

        try:
            arduinoData_float = float(arduinoData_string)   # Convert to float
            dataList.append(arduinoData_float)              # Add to the list holding the fixed number of points to animate

        except:                                             # Pass if data point is bad                               
            pass

        dataList = dataList[-50:]                           # Fix the list size so that the animation plot 'window' is x number of points
        
        ax.clear()                                          # Clear last data frame
        
        self.getPlotFormat()
        ax.plot(dataList)                                   # Plot new data frame
        

    def getPlotFormat(self):
        ax.set_ylim([0, 4100])                              # Set Y axis limit of plot
        ax.set_title("Data")                        # Set title of figure
        ax.set_ylabel("Value")                              # Set title of y axis

dataList = []                                           # Create empty list variable for later use
                                                        
fig = plt.figure()                                      # Create Matplotlib plots fig is the 'higher level' plot window
ax = fig.add_subplot(111)                               # Add subplot to main fig window
ln, = plt.plot([], [], 'r-')

realTimePlot = AnimationPlot()
time.sleep(2)                                           
                                                                  
ani = animation.FuncAnimation(fig, realTimePlot.animate, frames=100, fargs=(dataList, ser), interval=1) 

plt.show()                                              
ser.close()