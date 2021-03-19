# windows-recovery-media
윈도우 복구 및 백업 미디어 프로그램

개발 환경: visual studio
언어: c++
설명: WindowsPE 환경에서 윈도우 백업과 복구를 하고자 할 때 사용하는 프로그램.

기존에 cmd에서 명령어를 직접 입력해도 실행이 가능하지만 UI를 만들어 사용자가 명령어를 따로 입력하지 않고도 사용할 수 있도록 프로그램 개발.

역할: 코딩, UI 제작



*실행 화면
(1) 복구 화면

![Recovery](https://user-images.githubusercontent.com/22833414/111716255-fa427d00-8898-11eb-9b50-924238ef55f6.PNG)

(2) 백업 화면 

![Backup](https://user-images.githubusercontent.com/22833414/111716260-fadb1380-8898-11eb-91fe-08acb5fd1cb3.PNG)




*복구 실행 방법

1. Select Path로 복구 하고자 하는 .FFU 파일 선택

2. Disk 리스트 중 복구 할 Disk 선택

3. Recovery 버튼 클릭




*백업 실행 방법

1. Select Path로 백업 파일 저장 경로로 간 후 이미 백업 할 .FFU파일이 있다면 선택 or 없다면 만들고자 하는 이름을 파일 이름(N) 칸에 입력

2. Disk 리스트 중 백업 할 Disk 선택

3. BackUp 버튼 클릭




*프로그램에 적용된 명령어 예시

백업: dism.exe /capture-ffu /imagefile:e:\test.ffu /CaptureDrive:\\.\PhysicalDrive0 /Name:Drive0

복구: dism.exe /apply-ffu /imagefile:backup.ffu /ApplyDrive:\\.\PhysicalDrive0
