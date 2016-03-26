%it uses the subject filter to filter every subject
%1 is for self-face 0 is for non self-face
function allSubjectsFilter(option)
    for subjects = 1:10
        SubjectFilter(subjects,option)
    end
end