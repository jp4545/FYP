str = "cca.csv"
a = 1
b = 0
for i = 1:2416
    ans = mod(i,2)
    if ans == 0
        dlmwrite(str,a,'delimiter',',','-append');
    else
        dlmwrite(str,b,'delimiter',',','-append');
    end
end
