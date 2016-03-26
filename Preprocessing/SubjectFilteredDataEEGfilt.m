function sfd = SubjectFilteredDataEEGfilt(subjectNumber, channel)
    %load the subject content
    content = subjectConstructor(subjectNumber);
    load(content);
    
    %data and baseline
    %they have been reshaped because the resulting form is 1x250x2000
    %instead of 250x2000
    data = Epoch_T(1,1).data(channel,1:250,1:2000);
    %data = reshape(data,[250 2000])';
    
    baseline = Epoch_T(2,1).data(channel,1:50,1:2000);
    %baseline = reshape(baseline,[50 2000]);
    
    %Arrays that contains the filtered data
    filteredData = zeros(2000,250);
    moreFilteredData = zeros(2000,250);
    holyFilteredData = zeros(2000,250);
    
    %We need to filter every stimulus
    for stimulusIterator = 1:2000
        baselineMean = mean(baseline(1,1:50,stimulusIterator))*ones(1,250);
        god = data(1,1:250,stimulusIterator)-baselineMean(1,1:250);
        %first the 0.1Hz to 100Hz filter is applied
        filteredData(stimulusIterator,1:250) = eegfilt(god, 250, 0.1, 100, 250, 80, 0, 'fir1', 0 );
        %then the notch filter
        moreFilteredData(stimulusIterator,1:250) = notchFilter(filteredData(stimulusIterator,1:250), 250, 50);
        %and finally the 30Hz filter
        holyFilteredData(stimulusIterator,1:250) = eegfilt(moreFilteredData(stimulusIterator,1:250), 250, 0, 30, 250, 80, 0, 'fir1', 0 );
    end
    
    %after the data is filtered...
    %The idea is to calculate the mean of each point, for that we need to
    %calculate the mean of every column on the 250x2000 matrix
    %the mean of each subject is on sfd 
    sfd = zeros(1,250);
    for samplingIterator = 1:250
        vector = holyFilteredData(1:2000,samplingIterator);
        %this is only necessary if 1x1x2000 happens
        %vector = reshape(vector,[1 stimulus]);
        vectorMean = mean(vector);
        sfd(samplingIterator) = vectorMean; 
    end
end
