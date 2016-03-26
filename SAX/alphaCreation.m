function alphaCreation()
    alphabet = char(zeros(1,52));
    for upper = 65:90
        alphabet(upper-65+1) = char(upper);
    end
    for lower = 97:122
        alphabet(lower-97+26+1) = char(lower);
    end
    for i=1:52
        fprintf('%c',alphabet(i));
    end
end