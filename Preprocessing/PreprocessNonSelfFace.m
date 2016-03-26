function pre = PreprocessNonSelfFace(Subject)
channelVector = [27 21 22 26 20 16 19 15 8 13 14 6 12 4 5 2 3 1];
    pre = zeros(18,2000,250);
    waitBar = waitbar(0,'Stating');
    for channel = 1:18
        waitbar(channel/18,waitBar,strcat('Currently at channel:',num2str(channel)));
        fdata = NonSubjectFilteredData(Subject,channelVector(channel));
        for data = 1:250
            pre(channel,1:2000,data) = fdata(1:2000,data);
        end
    end
    waitbar(1, waitBar,'Finished');
end