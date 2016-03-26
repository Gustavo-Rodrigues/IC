function [y] = ts_m_sax_demo()

data = [0 cumsum(1 .* randn(1, 1024 - 1))];
flg_pause = 1;
alphabet_len = 10;
segment_len = 4;
y = ts_f_sax_demo(data, flg_pause, alphabet_len, segment_len);

end