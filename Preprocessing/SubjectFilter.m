%basically the channel filter is re-used to filter all the data of a subject
%again 1 is for self-face and 0 for non self-face 
function matrix = SubjectFilter(Subject,option)
   %those are the 18 channels
    channelVector = [27 21 22 26 20 16 19 15 8 13 14 6 12 4 5 2 3 1];
    matrix = zeros(18,2000,250);
    waitBar = waitbar(0,'Stating');
    for channel = 1:18
        waitbar(channel/18,waitBar,strcat('Currently at channel:',num2str(channel)));
        data = ChannelFilter(Subject,channelVector(channel),option);
        for idata = 1:250
            matrix(channel,1:2000,idata) = data(1:2000,idata);
        end
    end
    waitbar(1,waitBar,'Saving');
    
    if option == 1
        string = 'SelfFaceSubject';
        pathName = '/home/gustavo/Documents/IC/Data/PreprocessedSelfFace';
    else
        string = 'NonSelfFaceSubject';
        pathName = '/home/gustavo/Documents/IC/Data/PreprocessedNonSelfFace';
    end
    currentSubject = num2str(int8(Subject));
    subject = strcat(string,currentSubject);
    fullFileName = fullfile(pathName,subject);
    save(fullFileName,'matrix');
    waitbar(1, waitBar,'Finished');
    close(waitBar);
end