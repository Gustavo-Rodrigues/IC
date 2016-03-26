function SubjectSAX(Subject,option)
    if option == 1
        string = 'SelfFaceSubject';
        pathName = '/home/gustavo/Documents/IC/Data/SelfFaceSAX';
    else
        string = 'NonSelfFaceSubject';
        pathName = '/home/gustavo/Documents/IC/Data/NonSelfFaceSAX';
    end
    
    currentSubject = num2str(int8(Subject));
    String = strcat(string,currentSubject);
    fullFileName = fullfile(pathName,String);
    file = fopen(fullFileName,'a+');
    for channels = 1:18
        data = ChannelSAX(Subject,channels,option);
        %25,32,125,2000
        fprintf(file,'\n');
        for a = 1:25
            for b = 1:32
                for c = 1:125
                    for d = 1:2000
                        fprintf(file,'%d',data(a,b,c,d));
                    end
                end
            end
        end
    end
     fclose(file);
end