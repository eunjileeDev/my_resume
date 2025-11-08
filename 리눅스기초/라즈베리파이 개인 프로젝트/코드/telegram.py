import time
import cv2
import logging
import asyncio
import RPi.GPIO as GPIO
from telegram import Update, Bot
from telegram.ext import ApplicationBuilder, CommandHandler, ContextTypes
import threading

# ======================== 설정 ========================
TOKEN = '7983105221:AAHS1oqMAmKKaviAco-4FDKbWKlVhD93JcY'  # 여기에 당신의 토큰을 안전하게 입력하세요

TRIG = 20
ECHO = 16
LED = 6
SWITCH = 21

# ======================== GPIO 초기화 ========================
GPIO.setmode(GPIO.BCM)
GPIO.setwarnings(False)

GPIO.setup(TRIG, GPIO.OUT)
GPIO.setup(ECHO, GPIO.IN)
GPIO.setup(LED, GPIO.OUT)
GPIO.setup(SWITCH, GPIO.IN, pull_up_down=GPIO.PUD_DOWN)

# ======================== 전역 변수 ========================
alert_mode = False
chat_id_cache = None
bot = Bot(token=TOKEN)

# ======================== 거리 측정 ========================
def measure_distance():
    GPIO.output(TRIG, False)
    time.sleep(0.1)

    GPIO.output(TRIG, True)
    time.sleep(0.00001)
    GPIO.output(TRIG, False)

    pulse_start = time.time()
    timeout = pulse_start + 0.04
    while GPIO.input(ECHO) == 0:
        pulse_start = time.time()
        if pulse_start > timeout:
            return 999

    pulse_end = time.time()
    while GPIO.input(ECHO) == 1:
        pulse_end = time.time()
        if pulse_end - pulse_start > 0.04:
            return 999

    pulse_duration = pulse_end - pulse_start
    distance = pulse_duration * 17000
    return round(distance, 1)

# ======================== 사진 촬영 ========================
def capture_photo():
    cam = cv2.VideoCapture(0)
    if not cam.isOpened():
        return None
    ret, frame = cam.read()
    cam.release()
    if ret:
        path = "/tmp/capture.jpg"
        cv2.imwrite(path, frame)
        return path
    return None

# ======================== LED 깜빡임 ========================
def led_blink():
    global alert_mode
    while alert_mode:
        GPIO.output(LED, True)
        time.sleep(0.5)
        GPIO.output(LED, False)
        time.sleep(0.5)
    GPIO.output(LED, False)

# ======================== Telegram 명령어 ========================
async def photo(update: Update, context: ContextTypes.DEFAULT_TYPE):
    global alert_mode, chat_id_cache
    chat_id_cache = update.effective_chat.id

    if alert_mode:
        await update.message.reply_text("⚠️ 이미 경고 모드입니다.")
        return

    dist = measure_distance()
    await update.message.reply_text(f"📏 현재 측정된 거리: {dist} cm")

    if dist <= 50:
        alert_mode = True
        await context.bot.send_message(chat_id=chat_id_cache, text=f"🚨 물체 감지됨! 거리: {dist} cm")
        path = capture_photo()
        if path:
            with open(photo_path, 'rb') as img:
                await context.bot.send_photo(chat_id=chat_id_cache, photo=img)
        threading.Thread(target=led_blink, daemon=True).start()
    else:
        await update.message.reply_text("🔍 경고 조건이 아닙니다.")

# ======================== 스위치 이벤트 처리 ========================
def gpio_switch_handler(channel):
    global alert_mode, chat_id_cache
    if alert_mode:
        alert_mode = False
        GPIO.output(LED, False)
        if chat_id_cache:
            try:
                asyncio.run(bot.send_message(chat_id=chat_id_cache, text="✅ 경고 모드 해제됨"))
            except Exception as e:
                print(f"Error sending message: {e}")

# ======================== 메인 ========================
def main():
    GPIO.add_event_detect(SWITCH, GPIO.RISING, callback=gpio_switch_handler, bouncetime=1000)

    app = ApplicationBuilder().token(TOKEN).build()
    app.add_handler(CommandHandler("start", photo))

    print("🤖 Telegram 봇 실행 중...")
    app.run_polling()

# ======================== 실행 ========================
if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        print("\n⛔ 종료합니다.")
    finally:
        alert_mode = False
        GPIO.output(LED, False)
        GPIO.cleanup()

