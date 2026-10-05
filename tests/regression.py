import subprocess,re
for n in [0,1,9,10,11,100,1000]:
    for _ in range(4):
        result=subprocess.check_output(["./bin/hogwarts",str(n)],text=True,timeout=15)
        ids=sorted(map(int,re.findall(r"Task (\d+) was completed!",result)))
        assert ids==list(range(1,n+1)), (n,ids)
for bad in ["-1","abc"]:
    assert subprocess.run(["./bin/hogwarts",bad],capture_output=True).returncode != 0
print("Exactly-once work and termination checks PASS")
