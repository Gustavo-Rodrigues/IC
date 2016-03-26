function lol = ChannelAverage(channel)
    waitBar = waitbar(0,'Stating');
    tempStorage = zeros(10,250);
    
    %store the average of every subject channel
    for i = 1:10
        waitbar(i/10,waitBar,strcat('Storing the average of the channel for each subject',num2str(i)));
        filteredDatax = SubjectFilteredDataEEGfilt(i,channel);
        tempStorage(i,1:250) = filteredDatax(1:250);
    end
    waitbar(1,waitBar,'Finished! Starting to calculate the average of the channel');
    %the average o the channel
    lol = zeros(1,250);
    for iterator = 1:250
        waitbar(iterator/250, waitBar,strcat('Calculating the average of the channel',num2str(iterator)));
        lol(iterator) = mean(tempStorage(1:10,iterator));
    end
    waitbar(1, waitBar,'Finished');

end