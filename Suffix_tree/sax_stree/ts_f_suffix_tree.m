function [y] = ts_f_suffix_tree(t, tt, dmax, minLengh)

if(nargin < 4)
    tt = 1;
    dmax = 5;
    minLengh = 6;
end

javaaddpath('ai_suffixtree.jar');

st = br.com.henriquespassos.ai_suffixtree.SuffixTree;
list = st.list(t, tt, dmax, minLengh);

y = cell(list.size(), 2);

for i = 1:list.size()
    aux = list.get(i - 1);
    y{i, 1} = aux.get(0); % suffix
    y{i, 2} = aux.get(1); % occur, diff, conf, actual, perfect, meanPer
end

end