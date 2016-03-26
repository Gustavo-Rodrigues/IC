%This fucntion applies the SAX on a channel
%1 is for self-face 0 is for non self-face
function ChannelSAX2(Subject,Channel,option)
    %1,2,5,10,25,50,125,250
    red = [17,18,19,21,23,25,28,31,36,42,50,63,83,125];
    alpha = char(zeros(1,52));
    for upper = 65:90
        alpha(upper-65+1) = char(upper);
    end
    for lower = 97:122
        alpha(lower-97+26+1) = char(lower);
    end
    if option == 1
        %load the data
        pathName = '/home/gustavo/Documents/IC/Data/PreprocessedSelfFace';
        string = 'SelfFaceSubject';
        extension = '.mat';
        currentSubject = num2str(int8(Subject));
        subject = strcat(string,currentSubject,extension);
        adress = '/home/gustavo/Documents/IC/Data/SelfFaceSAX';
    else
        %load the data
        pathName = '/home/gustavo/Documents/IC/Data/PreprocessedNonSelfFace';
        string = 'NonSelfFaceSubject';
        extension = '.mat';
        currentSubject = num2str(int8(Subject));
        subject = strcat(string,currentSubject,extension);
        adress = '/home/gustavo/Documents/IC/Data/NonSelfFaceSAX';
    end
    %load
    fullFileName = fullfile(pathName,subject);
    load(fullFileName);
    %write
    sub = num2str(int8(Subject));
    name = strcat(string,sub,'Channel',num2str(int8(Channel)));
    fileName = fullfile(adress,name);
    
    file = fopen(fileName,'w');
    waitBar = waitbar(0,'Starting');
    %10 subjects, 18 channels, 14 PAA, 13 alphabets, 2000 stimulus   
    data = zeros(2000,250);
    for atribuition = 1:250
        data(1:2000,atribuition) = matrix(Channel,1:2000,atribuition);
    end
    
    %10->15->20...125
    for reduction = 1:14
        %2->4->8->16...64
        for alphabet = 4:4:52
            waitbar(alphabet/64,waitBar,strcat('Subject:',num2str(Subject),'Current reduction:',num2str(reduction), ' of 14.', ' alphabet:',num2str(alphabet)));
            for i = 1:2000
                %temp = ts_f_sax(data(i,1:250),250,reduction,alphabet);
                %k = size(temp);
                content = ts_f_sax(data(i,1:250),250,red(1,reduction),alphabet);
                Size = size(content);
                for o = 1:125
                    if o > Size(1,2);
                        break;
                    else
                        fprintf( file,'%s',alpha(content(1,o)) );
                    end
                end
                fprintf(file,'\n');
            end
        end
    end
    waitbar(1, waitBar,'Finished');
    close(waitBar);
    fclose(file);
end