function [y] = ts_m_sax()

data = [0 cumsum(1 .* randn(1, 1024 - 1))];
N = size(data, 2);
n = 10;
plot(data);
y = ts_f_sax(data, N, n, 20);

end