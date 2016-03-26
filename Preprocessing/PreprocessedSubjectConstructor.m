%construct the word subjectX.mat
function subject = PreprocessedSubjectConstructor(subjectNumber)
  string = 'PreprocessedSubject';
  extension = '.mat';
  currentSubject = num2str(int8(subjectNumber));
  tempString = strcat(string,currentSubject);
  subject = strcat(tempString,extension);
end