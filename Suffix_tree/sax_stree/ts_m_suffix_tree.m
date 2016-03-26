function [y] = ts_m_suffix_tree()

t = 'abcabbabb$';
y = ts_f_suffix_tree(t);

for i = 1:size(y, 1);
    fprintf('***\nletra = %s\n', y{i, 1});
    
    fprintf('occur = %s\n', mat2str(y{i, 2}(1, :)));
    fprintf('diff = %s\n', mat2str(y{i, 2}(2, :)));
    fprintf('conf = %s\n', mat2str(y{i, 2}(3, :)));
    fprintf('actual = %s\n', mat2str(y{i, 2}(4, :)));
    fprintf('perfect = %s\n', mat2str(y{i, 2}(5, :)));
    fprintf('mean per = %s\n', mat2str(y{i, 2}(6, :)))
    
    fprintf('***\n');
end

end