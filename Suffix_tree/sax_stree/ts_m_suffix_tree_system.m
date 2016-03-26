function ts_m_suffix_tree_system()
t = 'abcabbabb$';
system(['java -cp ai_suffixtree.jar br.com.henriquespassos.ai_suffixtree.SuffixTree -s ' t]);
end