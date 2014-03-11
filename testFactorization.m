clear all; close all; clc;
v = rand(1000,40);
w = rand(1000,5);
h = rand(5,40);
cost = 0; new_cost = 100;
while (abs(cost - new_cost) > 0.1)
    prod = w * h;
    new_w = w .* ((v ./ prod) * h');
    new_w = new_w ./ repmat(sum(new_w, 2), 1, size(new_w, 2));
    new_h = (w' * (v ./ prod)) .* h;
    w = new_w;
    h = new_h;
    cost = new_cost;
    new_cost = sum(sum(v .* log(w * h) - w * h));
end