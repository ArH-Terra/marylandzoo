clear; close all; clc;

% read data
data = readmatrix('program.xlsx', 'Range',1);

% load data
feeder_ID = data(1,2);

detector_num = data(3,2);
detector_IDs = nan(1, detector_num);
detector_timeouts = nan(1, detector_num);
detector_distances = nan(1, detector_num);
for i = 1:detector_num
    detector_IDs(i) = data(4,i+1);
    detector_timeouts(i) = data(5,i+1);
    detector_distances(i) = data(6,i+1);
end

% generate output string
output_string = "<";
output_string = output_string + num2str(feeder_ID);
output_string = output_string + " " + num2str(detector_num);
for i = 1:detector_num
    output_string = output_string + " " + num2str(detector_IDs(i));
    output_string = output_string + " " + num2str(detector_timeouts(i));
    output_string = output_string + " " + num2str(detector_distances(i));
end
output_string = output_string + ">";
disp(output_string);