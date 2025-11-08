const apiKey = '71b27ba5702e4d4df88c21d0e9d5782e';
const apiUrl = 'https://dapi.kakao.com/v3/search/book';

async function searchBooks() {
    const keyword = document.getElementById('keyword').value; //DOM html element
    if (!keyword) {     //잘못된 동작에 대한 에러 핸들링
        alert('키워드를 입력하세요');
        return;
    }

    try {
        const response = await axios.get(apiUrl, {
            headers: {
                'Authorization': `KakaoAK ${apiKey}`
            },
            params: {
                query: keyword,
                size: 1
            }
        });

        if (response.status === 200) {
            if (response.data.documents.length > 0) {
                displayResults(response.data.documents[0]);
            } else {
                alert('검색 결과가 없습니다.'); // 검색 결과가 없는 경우 에러 처리
            }
        } else {
            console.error(`Error: ${response.status} - ${response.statusText}`);
            alert('네트워크 오류가 발생했습니다. 잠시 후 다시 시도해주세요.'); // 네트워크 오류가 발생한 경우 에러 처리
        }
    } catch (error) {
        console.error('Error fetching data:', error);
        alert('네트워크 오류가 발생했습니다. 잠시 후 다시 시도해주세요.'); // 네트워크 오류가 발생한 경우 에러 처리
    }
}

    function displayResults(book) {
        const card = document.querySelector('.card');
        const img = card.querySelector('.card-img');
        const title = card.querySelector('.card-title');
        const date = card.querySelector('.card-date');
        const detail = card.querySelector('.card-detail');
    
        img.src = book.thumbnail;
        img.alt = book.title;
        title.textContent = book.title;
        date.textContent = book.authors.join(', ');
        detail.textContent = book.contents;
    
        card.addEventListener('click', () => {
            window.open(book.url, '_blank');
        });
    }

