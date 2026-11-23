# 친구에게 브라우저로 게임 보내기 (출시 전 비공개)

친구는 아무것도 설치하지 않고 브라우저(Windows/Mac/폰)에서 바로 플레이합니다.
방법은 두 가지입니다.

## 방법 A. itch.io 비공개 페이지 (추천)

링크와 비밀번호만 보내면 됩니다. 검색·목록에 안 나옵니다.

### 1. 계정 만들기 (Jiwon이 직접)
1. https://itch.io/register 에서 가입합니다.
2. 이메일 인증을 끝냅니다.

### 2. 프로젝트 만들기
1. 오른쪽 위 메뉴에서 **Upload new project** (또는 Dashboard → **Create new project**).
2. **Title**: `Paris Disparu`
3. **Kind of project** (문서 표기: Kind Of Game): **HTML** 선택.
4. **Uploads**: **Upload files** → `paris-disparu-web.zip` 선택.
   업로드 후 파일 옆의 **This file will be played in the browser** 를 체크합니다.
5. **Embed options**
   - **Embed in page**, 크기 **1000 × 820** (이 크기에서 스크롤 없이 전부 보이는 것을 확인함)
   - **Mobile friendly** 체크 (폰에서 전체화면으로 실행)
   - **Fullscreen button** 체크
   - **Click to play**는 켜 둔 채로 둡니다 (소리 재생에 클릭이 필요)

### 3. 비공개로 설정
1. 같은 편집 페이지 아래 **Visibility & access** 에서 **Restricted** 선택.
2. **Also allow a password to view page** 를 켜고 비밀번호를 입력합니다.
3. **Save** 를 누릅니다.

### 4. 친구에게 보내기
- 페이지 주소(예: `https://<아이디>.itch.io/paris-disparu`)와 비밀번호를 보냅니다.
- 친구가 페이지를 열면 비밀번호를 묻습니다. 주소 끝에 `?password=비밀번호` 를 붙이면 입력 없이 열립니다 (itch.io 관리자 답변 기준).

### 근거와 확인 못 한 부분
- 공개 상태 이름(Draft / Restricted / Public)과 "Also allow a password to view page"는 itch.io 공식 문서 https://itch.io/docs/creators/access-control 에서 확인했습니다.
- ZIP 조건(index.html 포함, 1,000개 파일 이하, 500MB 이하), Embed options, Mobile friendly 설명은 https://itch.io/docs/creators/html5 에서 확인했습니다. 이 zip은 파일 5개, 약 90KB입니다.
- `?password=` 와 "Restricted + 비밀번호를 추천한다"는 itch.io 관리자(leafo)의 포럼 답변입니다: https://itch.io/post/493498 (약 8년 전 글).
- **확인 못 함**: 친구가 itch.io 계정 없이 비밀번호만으로 볼 수 있는지 공식 문서에 명시가 없습니다. 메뉴 이름(Upload new project, This file will be played in the browser)은 화면이 바뀌었을 수 있습니다. 다르면 스크린샷을 보내 주세요.
- HTML 게임은 itch.io에서 기부만 받을 수 있고, 판매하려면 다운로드 판으로 따로 올려야 합니다 (위 html5 문서). 지금 단계(친구 테스트)와는 무관합니다.

## 방법 B. zip 파일을 그대로 보내기 (계정 불필요)

1. `paris-disparu-web.zip` 을 카카오톡/메일/드라이브로 보냅니다.
2. 친구는 **압축을 전부 푼 뒤** 폴더 안의 `index.html` 을 더블클릭합니다.
   (zip 안에서 바로 열면 다른 파일을 못 찾아 실행되지 않습니다.)
3. 인터넷 연결도 필요 없습니다 (로컬 파일로 실행되는 것을 자동 테스트로 확인).

## 공통 주의
- 게임 ROM이 파일 안에 들어 있어서, 받은 사람은 ROM을 꺼낼 수 있습니다. 믿을 수 있는 친구에게만 보내세요.
- 저장: 게임 화면 아래 **Save / Load** 버튼 또는 F6 / F9. 저장은 그 브라우저에만 남습니다.
- 조작: 방향키 이동, X = A(대화·확인), Z = B(닫기), Enter = START(수첩). 폰은 화면 아래 버튼.
- 최신 zip은 GitHub Actions의 **web** 실행 결과(Artifacts → `paris-disparu-web`)에서도 받을 수 있습니다.
