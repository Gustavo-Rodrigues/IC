function sca = SubjectChannels(subject)
    %channels
    waitBar = waitbar(0,'Stating');
    channelVector = [27 21 22 26 20 16 19 15 8 13 14 6 12 4 5 2 3 1];
    sca = zeros(18,250);
    for channels = 1:18
        waitbar(channels/18,waitBar,strcat('Currently at channel:',num2str(channels)));
        sca(channels,1:250) = SubjectFilteredDataEEGfilt(subject,channelVector(channels));
    end
    waitbar(1, waitBar,'Finished');

end