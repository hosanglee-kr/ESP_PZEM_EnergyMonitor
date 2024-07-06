
# git clone
```
git clone https://github.com/vortigont/espem ESP_PZEM_EnergyMonitoring
```

cd ESP_PZEM_EnergyMonitoring

git remote -v

git remote remove origin

rm -rf .git

git init
git add .
git commit -m "initial commit"

# branch 이름 변경
git branch -m main

# 3. gitnweb에서
#    신규Repository생성

# 4. Git 저장소 연결 후 강제 push
git remote add origin https://github.com/hosanglee-kr/ESP_PZEM_EnergyMonitoring.git
git push -u --force origin main


git fetch
git branch -r

git switch dev1


--------------------------

1. 서브트리로 사용할 원격 저장소 추가
#git remote add <원격 저장소의 이름> <원격 저장소의 주소>

git remote add pzem-edl https://github.com/hosanglee-kr/pzem-edl.git

2. 새로운 원격 저장소의 브랜치를 서브트리로 추가.
# git subtree add --prefix lib gitsubtree-lib master
# git subtree add --prefix <클론할 폴더> <원격 저장소의 이름> <브랜치 이름>

git subtree add --prefix lib_pzem-edl_main pzem-edl main

3. 서브트리를 원격에서 내려받기(pull)
git subtree pull --prefix lib_pzem-edl_main pzem-edl main

4. 서브트리를 원격에 올리기(push)
git subtree push --prefix lib_pzem-edl_main pzem-edl dev1