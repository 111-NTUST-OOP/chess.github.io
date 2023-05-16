const NUM_SONGS = 6;
var songIdx = 0;
var currentSong;

function nextSong() {
  if (currentSong) {
    currentSong.pause();
  }
  songIdx = (songIdx + 1) % (NUM_SONGS + 1)
  if (songIdx) {
    currentSong = new Audio(`./music/${songIdx}.mp3`);
    currentSong.addEventListener("ended", function() {
      nextSong();
    });
    currentSong.play();
  }
}



function showAlert(msg) {
  var alertBox = document.createElement("div");
  alertBox.className = "alert";
  alertBox.innerHTML = msg;
  document.body.appendChild(alertBox);
  setTimeout(function() {
    alertBox.remove();
  }, 3000);
}

const logToConsole = console.log;

function logToTextArea(msg) {
  const logArea = document.getElementById('log-area');
  logArea.value += msg + '\n';
  last10 = logArea.value.split('\n').splice(-11);
  logArea.value = last10.join('\n');
};

console.log = function(msg, alertOnSpecialMsg = true) {
  if (alertOnSpecialMsg && !["White to move", "Black to move", "FEN Copied!", "Invalid FEN!"].includes(msg)) {
    clearLog();
    showAlert(msg);
    stopTimer();
  }
  logToConsole(msg);
  logToTextArea(msg);
};

function clearLog() {
  document.getElementById('log-area').value = "";
}


var timerInterval;

function switchTimer() {
  const gameState = Module.getGameState(fen);
  let timer;
  
  if (gameState === "White to move") {
    timer = document.querySelector("#white-timer");
  } else if (gameState === "Black to move") {
    timer = document.querySelector("#black-timer");
  } else {
    return;
  }
  
  stopTimer();
  startTimer(timer);
  
  timer.classList.add("active");
}

function startTimer(timer) {
  timerInterval = setInterval(() => {
    const time = timer.innerHTML.split(':');
    let minutes = parseInt(time[0]);
    let seconds = parseInt(time[1]);

    if (seconds === 0) {
      if (minutes === 0) {
        clearInterval(timerInterval);
        let color = timer.id.split('-')[0];
        let msg = `Timeout: ${color === "white" ? "Black" : "White"} wins`;
        console.log(msg);
        return;
      } else {
        minutes--;
        seconds = 59;
      }
    } else {
      seconds--;
    }

    timer.innerHTML = `${minutes.toString().padStart(2, '0')}:${seconds.toString().padStart(2, '0')}`;
  }, 1000);
}

function stopTimer() {
  const timer = document.querySelector(".active");
  if (timer) {
    clearInterval(timerInterval);
    timer.classList.remove("active");
  }
}

function resetTimer() {
  stopTimer();
  document.querySelector("#white-timer").innerHTML = "10:00"
  document.querySelector("#black-timer").innerHTML = "10:00"
}



var fen;
var fens = [];
var fenIdx = -1;
var promoting = false;
var promotingSquare = "";

document.addEventListener('DOMContentLoaded', function() {
  document.querySelector("#chessboard").querySelectorAll('*').forEach(function(square) {
    square.addEventListener("mouseup", removeHighlights);
    square.addEventListener("click", click);
    square.addEventListener('dragstart', dragStart);
    square.addEventListener('dragover', dragOver);
    square.addEventListener('drop', drop);
  });
  nextSong();
});

Module['onRuntimeInitialized'] = function() {
  replay();
};

function undo() {
  cancelPromotion();
  if (fenIdx > 0) {
    clearLog();
    updateBoard(fens[--fenIdx], save = false);
    switchTimer();
  }
}

function redo() {
  cancelPromotion();
  if (fenIdx < fens.length - 1) {
    clearLog();
    updateBoard(fens[++fenIdx], save = false);
    switchTimer();
  }
}

function replay(fen_ = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1") {
  clearLog();
  fens = [];
  fenIdx = -1;
  updateBoard(fen_);
  resetTimer();
}

function addTargetHighlights(fen, sourceSquareId) {
  const targetSquares = Module.getValidTargetSquares(fen, sourceSquareId).split('\0');
  if (targetSquares[0].length === 2) {
    targetSquares.forEach(function(square) {
      document.getElementById(square).classList.add("highlight");
    });
  }
}

function removeHighlights() {
  document.querySelector('#chessboard').querySelectorAll('*').forEach(function(square) {
    square.classList.remove("highlight");
  });
}

function movePiece(fen, move) {
  if (move.length != 4) return false;
  const source = document.getElementById(move.substr(0, 2));
  const target = document.getElementById(move.substr(2, 2));
  
  if (source.className !== 'empty-square' && Module.isValidMove(fen, move)) {
    if (source.classList.contains("white-pawn") && target.id[1] === '8') {
      source.className = "empty-square";
      promotePiece(target.id, "white", move);
    } else if (source.classList.contains("black-pawn") && target.id[1] === '1') {
      source.className = "empty-square";
      promotePiece(target.id, "black", move);
    } else {
      fen = Module.getNextFEN(fen, move);
      updateBoard(fen);
    }
    switchTimer();
    return true;
  }
  return false;
}

var prevClickedSquareId = "";

function click(event) {
  if (event.target.id !== promotingSquare) {
    cancelPromotion();
    if (!movePiece(fen, prevClickedSquareId + event.target.id)) {
      prevClickedSquareId = event.target.id;
      addTargetHighlights(fen, event.target.id);
    }
  }
}

function dragStart(event) {
  removeHighlights();
  cancelPromotion();
  click(event);
  event.dataTransfer.setData("text/plain", event.target.id);
}

function dragOver(event) {
  event.preventDefault();
}

function drop(event) {
  event.preventDefault();
  removeHighlights();
  movePiece(fen, event.dataTransfer.getData('text/plain') + event.target.id);
}

function updateBoard(fen_, save = true, logGameState = true) {
  cancelPromotion();
  const chessPieces = Module.fenToHtmlClassNames(fen_).split('\0');
  if (chessPieces.length === 64) {
    if (save) {
      fens.push(fen_);
      ++fenIdx;
    }
    if (logGameState) {
      console.log(Module.getGameState(fen_));
    }
    fen = fen_;
    document.getElementById("fen").value = fen;
    document.querySelector('#chessboard').querySelectorAll('*').forEach(function(square, index) {
      square.className = chessPieces[index];
    });
    return true;
  }
  document.getElementById("fen").value = fen;
  return false;
}

function cancelPromotion() {
  if (promoting) {
    promoting = false;
    promotingSquare = ""
    updateBoard(fen, save = false, logGameState = false);
  }
}

function promotePiece(squareId, color, move) {
  setTimeout(function() {
    promoting = true;
    promotingSquare = squareId;
    document.getElementById(squareId).className = "piece " + color + "-promotion-menu";
    document.querySelector("#" + squareId).addEventListener('click', function(event) {
      const x = event.offsetX;
      const y = event.offsetY;
      const width = event.target.width;
      const height = event.target.height;
      let newPiece = '';
      if (x < width / 2 && y < height / 2) {
        newPiece = "knight";
        move += (color == "white" ? 'N' : 'n');
      } else if (x >= width / 2 && y < height / 2) {
        newPiece = "bishop";
        move += (color == "white" ? 'B' : 'b');
      } else if (x < width / 2 && y >= height / 2) {
        newPiece = "rook";
        move += (color == "white" ? 'R' : 'r');
      } else {
        newPiece = "queen";
        move += (color == "white" ? 'Q' : 'q');
      }
      if (promoting) {
        updateBoard(Module.getNextFEN(fen, move));
        switchTimer();
        promoting = false;
        promotingSquare = "";
      }
    }, { once: true });
  }, 50);
}

function handleFenSubmit(event) {
  event.preventDefault();
  if (!updateBoard(event.target.fen.value, save = true, logGameState = false)) {
    console.log("Invalid FEN!");
  } else {
    replay(event.target.fen.value);
  }
}

function copyFenToClipboard() {
  navigator.clipboard.writeText(fen);
  console.log("FEN Copied!");
}
