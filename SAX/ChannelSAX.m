%This fucntion applies the SAX on a channel
%1 is for self-face 0 is for non self-face
function TheMatrix = ChannelSAX(Subject,Channel,option)
    
    if option == 1
        %load the data
        pathName = '/home/gustavo/Documents/IC/Data/PreprocessedSelfFace';
        string = 'SelfFaceSubject';
        extension = '.mat';
        currentSubject = num2str(int8(Subject));
        subject = strcat(string,currentSubject,extension);
    else
        %load the data
        pathName = '/home/gustavo/Documents/IC/Data/PreprocessedNonSelfFace';
        string = 'NonSelfFaceSubject';
        extension = '.mat';
        currentSubject = num2str(int8(Subject));
        subject = strcat(string,currentSubject,extension);
    end
    
    fullFileName = fullfile(pathName,subject);
    load(fullFileName);
    
    waitBar = waitbar(0,'Stating');
    %10 subjects, 18 channels, 25 PAA == 10->15->20->25...125, 32 alphabets == 2->4->8->16...,2000 stimulus 
    %250<-this should be variable, so its inicializated with -1.  
    TheMatrix = (ones(25,32,125,2000)*-1);
    data = zeros(2000,250);
    for atribuition = 1:250
        waitbar(atribuition/250,waitBar,strcat('Aquiring the data:',num2str(atribuition)));
        data(1:2000,atribuition) = matrix(Channel,1:2000,atribuition);
    end
    %10->15->20...125
    for reduction = 5:5:125
        waitbar(reduction/125,waitBar,strcat('Current reduction:',num2str(reduction)));
        %2->4->8->16...64
        for alphabet = 2:2:64
            for i = 1:2000
                %temp = ts_f_sax(data(i,1:250),250,reduction,alphabet);
                %k = size(temp);
                TheMatrix(reduction/5,alphabet/2,1:reduction,i) = ts_f_sax(data(i,1:250),250,reduction,alphabet);
            end
        end
    end
    waitbar(1, waitBar,'Finished');
    close(waitBar);
end