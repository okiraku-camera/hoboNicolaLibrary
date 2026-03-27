/*
 * hobo_webhid.js , WebHID client script for hoboNicola devices.
 * Handles connection, parameter read/write, and NVM dump operations.
 * 
 * Copyright (c) 2026 Takeshi Higasa, okiraku-camera.tokyo
 * SPDX-License-Identifier: MIT
 */

const CONNECT_FILTERS = [{ usagePage: 0xff85, usage: 0x51 }];
const EXPECTED_VERSION = 0x1800;
const REPORT_ID_SEND = 5;
const CMD_GET_VERSION = 0x01;
const CMD_READ_NVM_BLOCK = 0x0a;
const CMD_GET_PARAMETER = 0x21;
const CMD_SET_PARAMETER = 0x20;

const MAX_PACKET_SIZE = 32;
const RESPONSE_TIMEOUT_MS = 1000;
const NVM_DUMP_TOTAL_BYTES = 80;
const NVM_DUMP_START_ADDR = 0;
const NVM_READ_CHUNK_BYTES = MAX_PACKET_SIZE - 2;

const connectButton = document.getElementById("connectButton");
const versionButton = document.getElementById("versionButton");
const loadButton = document.getElementById("loadButton");
const sendButton = document.getElementById("sendButton");
const readNvmButton = document.getElementById("readNvmButton");
const clearButton = document.getElementById("clearButton");
const debugToggle = document.getElementById("debugToggle");
const statusArea = document.getElementById("status");

const inputIds = ["param1", "param2", "param3", "param4"];

let device = null;
let versionConfirmed = false;
let pendingResponse = null;

function isCurrentDevice(target) {
  return Boolean(device && target && device === target);
}

function appendStatus(message) {
  const time = new Date().toLocaleTimeString();
  statusArea.textContent += `\n[${time}] ${message}`;
}

function isDebugEnabled() {
  return Boolean(debugToggle && debugToggle.checked);
}

function getValidatedValue(id) {
  const element = document.getElementById(id);
  const value = Number.parseInt(element.value, 10);
  if (!Number.isInteger(value) || value < 50 || value > 500) {
    throw new Error(`${id} は 50 から 500 の整数を入力してください。`);
  }
  return value;
}

function writeU16LE(view, offset, value) {
  view[offset] = value & 0xff;
  view[offset + 1] = (value >> 8) & 0xff;
}

function readI16LE(view, offset) {
  return new DataView(view.buffer, view.byteOffset, view.byteLength).getInt16(offset, true);
}

function formatHexBytes(bytes) {
  return Array.from(bytes)
    .map((b) => `0x${b.toString(16).padStart(2, "0")}`)
    .join(" ");
}

function delay(ms) {
  return new Promise((resolve) => setTimeout(resolve, ms));
}

function updateButtons() {
  const canOperate = Boolean(device && device.opened && versionConfirmed);
  versionButton.disabled = !Boolean(device && device.opened);
  loadButton.disabled = !canOperate;
  sendButton.disabled = !canOperate;
  readNvmButton.disabled = !canOperate;
}

function formatHexLines(bytes, lineWidth = 16, startAddress = 0) {
  const lines = [];
  for (let i = 0; i < bytes.length; i += lineWidth) {
    const line = bytes.slice(i, i + lineWidth);
    const address = `0x${(startAddress + i).toString(16).padStart(4, "0")}`;
    lines.push(
      `${address} : ${Array.from(line)
        .map((b) => b.toString(16).padStart(2, "0"))
        .join(" ")}`
    );
  }
  return lines.join("\n");
}

function resolvePendingResponse(bytes) {
  if (!pendingResponse) {
    return false;
  }
  clearTimeout(pendingResponse.timeoutId);
  pendingResponse.resolve(bytes);
  pendingResponse = null;
  return true;
}

function rejectPendingResponse(error) {
  if (!pendingResponse) {
    return;
  }
  clearTimeout(pendingResponse.timeoutId);
  pendingResponse.reject(error);
  pendingResponse = null;
}

async function resetConnectionState(reason, targetDevice = device) {
  rejectPendingResponse(new Error(reason || "デバイス接続が切断されました。"));

  if (targetDevice) {
    targetDevice.removeEventListener("inputreport", onInputReport);
    if (targetDevice.opened) {
      try {
        await targetDevice.close();
      } catch (error) {
        console.warn("WebHID close error", error);
      }
    }
  }

  if (!targetDevice || isCurrentDevice(targetDevice)) {
    device = null;
    versionConfirmed = false;
  }
  updateButtons();
}

function onHidDisconnect(event) {
  const disconnected = event.device;
  if (!isCurrentDevice(disconnected)) {
    return;
  }

  appendStatus("デバイス切断を検出しました。接続状態を解除します。");
  resetConnectionState("デバイスが切断されたため、応答待機を中断しました。", disconnected).catch((error) => {
    console.warn("disconnect cleanup error", error);
  });
}

function onInputReport(event) {
  const reportId = event.reportId;
  const bytes = new Uint8Array(event.data.buffer);
  const hex = formatHexBytes(bytes);

  if (isDebugEnabled()) {
    appendStatus(`受信 reportId=${reportId}, size=${bytes.length}, bytes=[${hex}]`);
  }
  console.log(`WebHID response reportId=${reportId} size=${bytes.length} hex=[${hex}]`);

  // コマンド応答待ちを解決するのは RAW レポートだけに限定する。
  if (reportId !== REPORT_ID_SEND) {
    return;
  }

  if (bytes.length > MAX_PACKET_SIZE) {
    rejectPendingResponse(new Error(`応答サイズ超過: ${bytes.length} バイト（最大 ${MAX_PACKET_SIZE}）`));
    return;
  }

  if (resolvePendingResponse(bytes)) {
    return;
  }
}

function buildTimeoutError() {
  return new Error(`応答タイムアウト（${RESPONSE_TIMEOUT_MS}ms）`);
}

function getResponseBodyForCommand(response, expectedCommand, minBodySize = 0) {
  if (response.length < 1) {
    throw new Error("応答が空です（コマンドバイトなし）。");
  }

  const responseCommand = response[0];
  if (responseCommand !== expectedCommand) {
    throw new Error(
      `応答コマンド不一致: expected=0x${expectedCommand.toString(16).padStart(2, "0")}, actual=0x${responseCommand.toString(16).padStart(2, "0")}`
    );
  }

  const body = response.slice(1);
  if (body.length < minBodySize) {
    throw new Error(`応答データが短すぎます（必要 ${minBodySize} バイト, 実際 ${body.length} バイト）。`);
  }

  return body;
}

async function sendCommandWithResponse(payload) {
  if (!device || !device.opened) {
    throw new Error("デバイスが接続されていません。");
  }

  if (!(payload instanceof Uint8Array)) {
    throw new Error("送信ペイロードは Uint8Array である必要があります。");
  }

  if (payload.length < 1) {
    throw new Error("先頭1バイトにコマンドコードが必要です。");
  }

  if (payload.length > MAX_PACKET_SIZE) {
    throw new Error(`送信サイズ超過: ${payload.length} バイト（最大 ${MAX_PACKET_SIZE}）`);
  }

  if (pendingResponse) {
    throw new Error("別コマンドの応答待機中です。");
  }

  const responsePromise = new Promise((resolve, reject) => {
    const timeoutId = setTimeout(() => {
      pendingResponse = null;
      const err = buildTimeoutError();
      console.warn(`WebHID timeout cmd=0x${payload[0].toString(16).padStart(2, "0")}: ${err.message}`);
      reject(err);
    }, RESPONSE_TIMEOUT_MS);

    pendingResponse = { resolve, reject, timeoutId };
  });

  // sendReport完了前にタイムアウトしても未捕捉 reject にならないようにする。
  responsePromise.catch(() => {});

  const payloadHex = formatHexBytes(payload);
  const cmdHex = `0x${payload[0].toString(16).padStart(2, "0")}`;
  if (isDebugEnabled()) {
    appendStatus(`送信ペイロード dump cmd=${cmdHex}, len=${payload.length}, hex=[${payloadHex}]`);
  }
  console.log(`WebHID payload dump cmd=${cmdHex} len=${payload.length} hex=[${payloadHex}]`);

  try {
    await device.sendReport(REPORT_ID_SEND, payload);
    if (isDebugEnabled()) {
      appendStatus(`送信 reportId=${REPORT_ID_SEND}, size=${payload.length}, bytes=[${formatHexBytes(payload)}]`);
    }
  } catch (error) {
    rejectPendingResponse(error);
    throw error;
  }

  return responsePromise;
}

async function connectDevice() {
  if (!navigator.hid) {
    throw new Error("このブラウザは WebHID に対応していません。");
  }

  const devices = await navigator.hid.requestDevice({ filters: CONNECT_FILTERS });
  if (!devices.length) {
    throw new Error("デバイスが選択されませんでした。");
  }

  device = devices[0];
  try {
    await device.open();
    device.addEventListener("inputreport", onInputReport);

    versionConfirmed = false;
    updateButtons();

    appendStatus(`接続完了: ${device.productName || "unknown device"}`);

    const versionCmd = new Uint8Array([CMD_GET_VERSION]);
    const response = await sendCommandWithResponse(versionCmd);

    const body = getResponseBodyForCommand(response, CMD_GET_VERSION, 2);
    const version = body[0] | (body[1] << 8);
    if (version !== EXPECTED_VERSION) {
      throw new Error(`バージョン不一致: 0x${version.toString(16).padStart(4, "0")}`);
    }

    versionConfirmed = true;
    appendStatus("バージョン確認成功");
    updateButtons();

    // 接続と同時にパラメータを取得
    try {
      await loadParameters();
    } catch (error) {
      appendStatus(`パラメータ取得エラー: ${error.message}`);
    }
  } catch (error) {
    await resetConnectionState("接続初期化中にエラーが発生しました。", device);
    throw error;
  }
}

async function loadParameters() {
  const response = await sendCommandWithResponse(new Uint8Array([CMD_GET_PARAMETER]));
  // body[0]: 項目数, body[1..]: 16ビット値の列
  const body = getResponseBodyForCommand(response, CMD_GET_PARAMETER, 1);
  const count = body[0];
  if (body.length < 1 + count * 2) {
    throw new Error(`応答データが短すぎます（count=${count}, 必要 ${1 + count * 2} バイト, 実際 ${body.length} バイト）。`);
  }
  const data = body.slice(1);
  const n = Math.min(count, inputIds.length);
  const values = Array.from({ length: n }, (_, index) => readI16LE(data, index * 2));

  inputIds.slice(0, n).forEach((id, index) => {
    document.getElementById(id).value = String(values[index]);
  });

  appendStatus(`パラメータ読込完了: count=${count}, [${values.join(", ")}]`);
}

async function sendParameters() {
  if (!device || !device.opened) {
    throw new Error("先にデバイスへ接続してください。");
  }
  
  const values = inputIds.map(getValidatedValue);
  const payload = new Uint8Array(2 + values.length * 2);

  payload[0] = CMD_SET_PARAMETER;
  payload[1] = values.length;
  values.forEach((value, index) => {
    writeU16LE(payload, 2 + index * 2, value);
  });

  const response = await sendCommandWithResponse(payload);
  const body = getResponseBodyForCommand(response, CMD_SET_PARAMETER, 0);
  appendStatus(`パラメータ送信完了: [${values.join(", ")}], 応答データサイズ=${body.length}`);
}

async function readNvmBlock(startAddress, byteCount) {
  if (!Number.isInteger(startAddress) || startAddress < 0 || startAddress > 0xff) {
    throw new Error("開始アドレスは 0 から 255 の整数にしてください。");
  }
  if (!Number.isInteger(byteCount) || byteCount < 1 || byteCount > NVM_READ_CHUNK_BYTES) {
    throw new Error(`読出しバイト数は 1 から ${NVM_READ_CHUNK_BYTES} の整数にしてください。`);
  }

  const payload = new Uint8Array([CMD_READ_NVM_BLOCK, startAddress, byteCount]);
  const response = await sendCommandWithResponse(payload);
  const body = getResponseBodyForCommand(response, CMD_READ_NVM_BLOCK, 1);

  const actualCount = body[0];
  if (body.length < 1 + actualCount) {
    throw new Error(`READ_NVM_BLOCK 応答不足: expected=${1 + actualCount}, actual=${body.length}`);
  }
  if (actualCount !== byteCount) {
    throw new Error(`READ_NVM_BLOCK 読出しサイズ不一致: requested=${byteCount}, actual=${actualCount}`);
  }

  return body.slice(1, 1 + actualCount);
}

async function readNvmDump80Bytes() {
  if (!device || !device.opened) {
    throw new Error("先にデバイスへ接続してください。");
  }

  const chunks = [];
  let readOffset = 0;
  while (readOffset < NVM_DUMP_TOTAL_BYTES) {
    const chunkSize = Math.min(NVM_READ_CHUNK_BYTES, NVM_DUMP_TOTAL_BYTES - readOffset);
    const chunk = await readNvmBlock(NVM_DUMP_START_ADDR + readOffset, chunkSize);
    chunks.push(chunk);
    readOffset += chunk.length;
  }

  const fullDump = new Uint8Array(NVM_DUMP_TOTAL_BYTES);
  let pos = 0;
  for (const chunk of chunks) {
    fullDump.set(chunk, pos);
    pos += chunk.length;
  }

  appendStatus(`NVM読出し完了: start=${NVM_DUMP_START_ADDR}, bytes=${fullDump.length}`);
  appendStatus(`NVM HEX:\n${formatHexLines(fullDump, 16, NVM_DUMP_START_ADDR)}`);
}

versionButton.addEventListener("click", async () => {
  try {
    const versionCmd = new Uint8Array([CMD_GET_VERSION]);
    const response = await sendCommandWithResponse(versionCmd);
    const body = getResponseBodyForCommand(response, CMD_GET_VERSION, 2);
    const version = body[0] | (body[1] << 8);
    appendStatus(`バージョン: 0x${version.toString(16).padStart(4, "0")}`);
  } catch (error) {
    appendStatus(`バージョン取得エラー: ${error.message}`);
  }
});

connectButton.addEventListener("click", async () => {
  try {
    if (device && device.opened) {
      appendStatus("既に接続済みです。");
      return;
    }
    await connectDevice();
  } catch (error) {
    appendStatus(`接続エラー: ${error.message}`);
    updateButtons();
  }
});

sendButton.addEventListener("click", async () => {
  try {
    await sendParameters();
  } catch (error) {
    appendStatus(`送信エラー: ${error.message}`);
  }
});

loadButton.addEventListener("click", async () => {
  try {
    await loadParameters();
  } catch (error) {
    appendStatus(`取得エラー: ${error.message}`);
  }
});

readNvmButton.addEventListener("click", async () => {
  try {
    await readNvmDump80Bytes();
  } catch (error) {
    appendStatus(`NVM読出しエラー: ${error.message}`);
  }
});

clearButton.addEventListener("click", () => {
  statusArea.textContent = "待機中...";
});

if (navigator.hid) {
  navigator.hid.addEventListener("disconnect", onHidDisconnect);
}

statusArea.textContent = "待機中...";
