// 입력 칸의 힌트 텍스트를 처리하는 함수
function handleInputHint() {
    var genreInput = document.getElementById("genreInput");

    genreInput.addEventListener("focus", function() {
        if (genreInput.value === "장르 입력") {
            genreInput.value = "";
        }
    });

    genreInput.addEventListener("blur", function() {
        if (genreInput.value === "") {
            genreInput.value = "장르 입력";
        }
    });
}
function genreButton() {
    const genreList = document.getElementById("genreList");
    genreList.classList.toggle("show");
}


// 장르명 코드로 변환
function convertGenreCode(input) {
    const genreCodes = {
        '연극': '005',
        '무용': '007',
        '뮤지컬': '008',
        '콘서트': '017',
        '클래식': '018',
        '재즈': '019',
        '크로스오버': '020',
        '복합장르': '021',
        '오페라': '001'
    };
    return genreCodes[input];
}


// API 요청 함수
async function fetchPerformances() {
    const genreInput = document.getElementById("genreInput").value; // DOM html element
    console.log('Genre Input:', genreInput); // 디버깅용 로그

    const genre = convertGenreCode(genreInput);
    console.log('Converted Genre Code:', genre); // 디버깅용 로그

    if (!genre) { // 잘못된 동작에 대한 에러 핸들링
        console.error('Invalid genre input');
        alert('유효하지 않은 장르입니다.');
        return;
    }

    const url = 'http://api.kcisa.kr/openapi/API_CCA_142/request';
    const queryParams = `?${encodeURIComponent('serviceKey')}=445e9106-4dd0-4795-8b9f-feb576f973e2` +
                        `&${encodeURIComponent('numOfRows')}=${encodeURIComponent('4')}` +
                        `&${encodeURIComponent('pageNo')}=${encodeURIComponent('1')}` +
                        `&${encodeURIComponent('infoTp')}=${encodeURIComponent(genre)}`;

    console.log('Request URL:', url + queryParams); // 디버깅용 로그

    const maxRetries = 40; // 최대 재시도 횟수
    const timeout = 5000; // 타임아웃 시간 (밀리초)

    for (let attempt = 1; attempt <= maxRetries; attempt++) {
        try {
            const response = await fetchWithTimeout(url + queryParams, { timeout });

            if (response.status === 200) {
                const text = await response.text();
                const parser = new DOMParser();
                const xmlDoc = parser.parseFromString(text, 'text/xml');
                const items = xmlDoc.getElementsByTagName('item'); // DOM html element

                // 각 카드 정보 설정
                if (items.length > 0) {
                    const firstItem = items[0];
                    document.querySelector('.card-title1').textContent = firstItem.getElementsByTagName('title')[0].textContent;
                    document.querySelector('.card-date1').textContent = firstItem.getElementsByTagName('regDate')[0].textContent.substring(0, 10);
                    document.querySelector('.card-detail1').textContent = firstItem.getElementsByTagName('description')[0].textContent;
                }
                if (items.length > 1) {
                    const secondItem = items[1];
                    document.querySelector('.card-title2').textContent = secondItem.getElementsByTagName('title')[0].textContent;
                    document.querySelector('.card-date2').textContent = secondItem.getElementsByTagName('regDate')[0].textContent.substring(0, 10);
                    document.querySelector('.card-detail2').textContent = secondItem.getElementsByTagName('description')[0].textContent;
                }
                if (items.length > 2) {
                    const thirdItem = items[2];
                    document.querySelector('.card-title3').textContent = thirdItem.getElementsByTagName('title')[0].textContent;
                    document.querySelector('.card-date3').textContent = thirdItem.getElementsByTagName('regDate')[0].textContent.substring(0, 10);
                    document.querySelector('.card-detail3').textContent = thirdItem.getElementsByTagName('description')[0].textContent;
                }
                if (items.length > 3) {
                    const fourthItem = items[3];
                    document.querySelector('.card-title4').textContent = fourthItem.getElementsByTagName('title')[0].textContent;
                    document.querySelector('.card-date4').textContent = fourthItem.getElementsByTagName('regDate')[0].textContent.substring(0, 10);
                    document.querySelector('.card-detail4').textContent = fourthItem.getElementsByTagName('description')[0].textContent;
                }
                break; // 성공적으로 데이터를 가져왔으므로 반복문 종료
            } else {
                console.error('Failed to fetch performances:', response.statusText);
                alert('공연 정보를 가져오는데 실패했습니다. 잠시 후 다시 시도해주세요.');
            }
        } catch (error) {
            console.error('Error fetching data (Attempt ' + attempt + '):', error);
            if (attempt === maxRetries) {
                alert('공연 정보를 가져오는데 실패했습니다. 잠시 후 다시 시도해주세요.');
            }
        }
    }
}

// fetchWithTimeout 함수 정의
async function fetchWithTimeout(resource, options) {
    const { timeout = 8000 } = options;

    const controller = new AbortController();
    const id = setTimeout(() => controller.abort(), timeout);
    const response = await fetch(resource, {
        ...options,
        signal: controller.signal
    });
    clearTimeout(id);

    return response;
}


// 예제 함수 호출
handleInputHint();

function copyContent() {
    var title = document.querySelector('.card-title1').innerText;
    var date = document.querySelector('.card-date1').innerText;
    var detail = document.querySelector('.card-detail1').innerText;
  
    var copyText = title + '\n' + date + '\n' + detail;
    
    var textarea = document.createElement('textarea');
    textarea.value = copyText;
    document.body.appendChild(textarea);
    textarea.select();
    document.execCommand('copy');
    document.body.removeChild(textarea);
    
    alert('내용이 복사되었습니다!');
  }







