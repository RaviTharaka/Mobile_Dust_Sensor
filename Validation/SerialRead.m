clear all;
close all;

sFile = 'Dataset.csv';

delete(instrfindall);   %pre-emptively close all ports
s1 = serial('COM6');    %define serial port to read the Arduino
s1.BaudRate=9600;     %define baud rate
    
fopen(s1);
s1.ReadAsyncMode = 'continuous';
readasync(s1);
    
while(s1.BytesAvailable <= 0)  %wait until Arduino outputs data 
   
end

mData = [];
for i=1:1000 %while if constant acquisition is needed. 
    sSerialData = fscanf(s1); %read sensor
    flushinput(s1);
    % = strsplit(sSerialData,'\n'); % same character as the Arduino code
   csvwrite(sFile,sSerialData);   % save the data to a CSV file
end

delete(instrfindall);    % close the serial port
%csvwrite(sFile,mData);   % save the data to a CSV file
 