clear all;

%% Input required date
startDate = '27-Feb-2018 00:00:00';
endDate   = '28-Feb-2018 02:50:00';
[dataNum, dataText] = xlsread('data.csv');

pm10 = dataNum(:,5);
pm25 = dataNum(:,6);

for i = 1 : size(pm10, 1)
    time(i,1) = datetime(dataText(i+1,7),'InputFormat','H:m:s''T''d:M:y');
end

t10  = timeseries(pm10,datestr(time));
t25  = timeseries(pm25,datestr(time));

%% Plot the graph
temp1 = getsampleusingtime(t10,startDate,endDate);
temp2 = getsampleusingtime(t25,startDate,endDate);

figure('units','normalized','outerposition',[0 0 1 1])
plot(temp1,'red');
hold on;
plot(temp2,'blue');
hold off;
title([startDate, ' ', endDate, '']);
legend('PM 10','PM 2.5');
ylim([0 200]);
