%this function filters the desired channel from a subject
%the option allows to chose if it's self-face or non self-face
%1 is for self-face and 0 for non-self face
function cf = ChannelFilter(subjectNumber,channel, option)
    %load the subject content
    pathName = '/home/gustavo/Documents/IC/Data/OriginalData';
    string = 'Subject';
    extension = '.mat';
    currentSubject = num2str(int8(subjectNumber));
    subject = strcat(string,currentSubject,extension);
    
    fullFileName = fullfile(pathName,subject);
    load(fullFileName);
    
    data = zeros(2000,250);
    
    if option == 1
        
        tempData = zeros(250,2000);
        tempBaseline = zeros(50,2000);
        
        %data and baseline
        for stimulus = 1:2000
            tempData(1:250,stimulus) = Epoch_T(1,1).data(channel,1:250,stimulus);
            tempBaseline(1:50,stimulus) = Epoch_T(2,1).data(channel,1:50,stimulus);
            baselineMean = mean(tempBaseline(1:50,stimulus))*ones(250,1);
            data(stimulus,1:250) = tempData(1:250,stimulus)- baselineMean; 
        end
        
        
    else
        %data and baseline
        tempData = zeros(250,200);
        tempBaseline = zeros(50,200);     
    
        atribuition = 0;
        for nonSub = 2:11
            for stimulus = 1:200
                tempData(1:250,stimulus) = Epoch_T(1,nonSub).data(channel,1:250,stimulus);
                tempBaseline(1:50,stimulus) = Epoch_T(2,nonSub).data(channel,1:50,stimulus);
                data(stimulus+atribuition,1:250) = tempData(1:250,stimulus)-((mean(tempBaseline(1:50,stimulus)))*ones(250,1));
            end
            atribuition = atribuition + 200;
        end
    end
    %Arrays that contains the filtered data
    filteredData = zeros(2000,250);
    filteredData2 = zeros(2000,250);
    filteredData3 = zeros(2000,250);
        
        
    %We need to filter every stimulus
    for stimulusIterator = 1:2000
        %first the 0.1Hz to 100Hz filter is applied
        filteredData(stimulusIterator,1:250) = eegfilt(data(stimulusIterator,1:250), 250, 0.1, 100, 250, 80, 0, 'fir1', 0 );
        %then the notch filter
        filteredData2(stimulusIterator,1:250) = notchFilter(filteredData(stimulusIterator,1:250), 250, 50);
        %and finally the 30Hz filter
        filteredData3(stimulusIterator,1:250) = eegfilt(filteredData2(stimulusIterator,1:250), 250, 0, 30, 250, 80, 0, 'fir1', 0 );
    end
    cf = filteredData3(1:2000,1:250);
end