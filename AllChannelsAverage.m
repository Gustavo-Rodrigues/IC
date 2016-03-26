 %calculate all channels the channel average
function totalChannelAverageVector = AllChannelsAverage()

    waitBar = waitbar(0,'Stating');
    
    channelVector = [27 21 22 26 20 16 19 15 8 13 14 6 12 4 5 2 3 1];
    %vector that has all channels average
    allChannelsAverageVector = zeros(18,250);
    
    for channelNumber = 1:18
        %creates a 10x250 vector to store every subject channel average
        tempChannelVector = zeros(10,250);
        for subIterator = 1:10
            waitbar(subIterator/10, waitBar,strcat('Current channel: ',num2str(channelNumber),'Current Subject: ',num2str(subIterator)));
            eachChannelAverage = SubjectFilteredDataEEGfilt(subIterator,channelVector(channelNumber));
            tempChannelVector(subIterator,1:250) = eachChannelAverage(1,1:250);
        end
        %creates a 1x250 vector that is the total channel average
        averageVector = zeros(1,250);
        for tempIterator = 1:250
            waitbar(tempIterator/250, waitBar,strcat('Calculating the average of the channel: ',num2str(tempIterator)));
            averageVector(1,tempIterator) = mean(tempChannelVector(1:10,tempIterator));
        end
        allChannelsAverageVector(channelNumber,1:250) = averageVector(1,1:250);
    end
    totalChannelAverageVector = allChannelsAverageVector;
    waitbar(1, waitBar,'Finished');
end






