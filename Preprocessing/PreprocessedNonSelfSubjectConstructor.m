function subject = PreprocessedNonSelfSubjectConstructor(subjectNumber)
  string = 'PreprocessedNonSelfSubject';
  extension = '.mat';
  currentSubject = num2str(int8(subjectNumber));
  tempString = strcat(string,currentSubject);
  subject = strcat(tempString,extension);
end