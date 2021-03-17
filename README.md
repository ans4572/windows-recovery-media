# windows-recovery-media
윈도우 복구 및 백업 미디어 프로그램

개발 환경: visual studio
언어: c++
설명: WindowsPE 환경에서 윈도우 백업과 복구를 하고자 할 때 사용하는 프로그램.
기존에 cmd에서 명령어를 직접 입력해도 실행이 가능하지만 UI를 만들어 사용자가 명령어를 따로 입력하지 않고도 사용할 수 있도록 프로그램 개발.
역할: 코딩, UI 제작

실행 명령어 예시
백업: dism.exe /capture-ffu /imagefile:e:\test.ffu /CaptureDrive:\\.\PhysicalDrive0 /Name:Drive0
복구: dism.exe /apply-ffu /imagefile:backup.ffu /ApplyDrive:\\.\PhysicalDrive0
