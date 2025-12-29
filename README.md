# VEDA Linux Programming Project

**Version** : v1.0.0  
**Author**  : 손현석 (Nakta), moneydon77@naver.com  
**Last Updated** : 2025-12-29  

라즈베리파이에서 TCP 프로토콜을 통해 LED, buzzer, cds, 7-segment 모듈을 제어하는 프로그램입니다. 서버(`main`)는 포트 60000을 열고  클라이언트(`client`)는 명령을 전송합니다. 런타임에는 `lib/libbuzzer.so`, `lib/libcds.so`, `lib/libseven.so`를 동적으로 로드합니다.

## 구성
- `main` : 서버 실행 파일
- `client` : 테스트용 클라이언트
- `lib/` : 런타임 로드되는 공유 라이브러리 3종
- `music/` : 음악 데이터 초기화 코드
- `include/` : 공용 헤더

## 빌드 (Ubuntu, 크로스 컴파일)
64비트 Pi 기준 `aarch64-linux-gnu-gcc`를 사용하도록 `Makefile`이 설정되어 있습니다. 32비트 Pi라면 `arm-linux-gnueabihf-gcc`로 수정해야 합니다. 크로스 컴파일을 하지 않을거라면 Makefile 에서 gcc로 수정하세요.

```sh
# 루트
make clean
make            # main, client, lib/*.so 생성
```

라이브러리만 다시 빌드하려면:
```sh
cd lib
make clean
make
```

## 배포 (라즈베리파이로 복사 예시)
```sh
# Pi 실행 경로 예: /home/pi/VEDA_Linux_Programming_Project
scp main lib/lib*.so pi@<pi-ip>:/라즈베리파이 경로/
```
- 실행 시 작업 디렉터리는 프로젝트 루트여야 하며 `./lib/*.so`가 함께 있어야 합니다.
- 권한 확인: `chmod 755 main lib/lib*.so`
- Pi에서 wiringPi가 설치되어 있어야 합니다.

## 실행 (라즈베리파이)
```sh
cd /라즈베리파이 경로
./main                      # 서버 시작 (포트 60000)
# 또는 라이브러리 경로를 명시
# LD_LIBRARY_PATH=./lib ./main  -  필요시 ~/.bashrc 파일에 작성
```

## 클라이언트 사용
```sh
./client <서버_IP>
```

연결 후 입력할 명령:
- `help` : 명령 목록 표시
- `LED ON|OFF|MAX|MID|MIN` : LED 켜기/끄기/밝기 조절
- `buzzer ON|OFF` : 음악 재생 온/오프
- `music 1|2|3|4` : 내장된 음악 선택
- `cds` : 조도값 조회
- `seg 0~9` : 7-Segment 카운트다운 실행

## 문제 해결
- `dlopen: permission denied`  
  - `lib/*.so`가 있는지, 실행 권한이 있는지, 프로젝트 루트에서 실행 중인지 확인.
  - `noexec` 마운트라면 `exec`가 허용된 경로로 옮겨 실행.
- `Exec format error`  
  - Pi 아키텍처(32/64비트)와 크로스 컴파일러/바이너리가 일치하는지 확인.
- 의존성 누락  
  - `ldd main`, `ldd lib/libbuzzer.so`로 `libwiringPi` 등 필요 so가 있는지 확인 후 설치.
