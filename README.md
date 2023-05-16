# Chess Project

https://111-ntust-oop.github.io/chess.github.io/chess.html

### Compile
* em++ -std=c++20 -lembind -o module.js module.cpp

## End conditions

### Win / Lose
| Checkmate | Resignation | Time out |
|:---------:|:-----------:|:--------:|
|✓          |✓            |✓        |

### Draw
| Threefold repetition | 50-move rule | Stalemate | Draw by agreement | Insufficient material |
|:--------------------:|:------------:|:---------:|:-----------------:|:---------------------:|
|                      |              |✓          |                   |                       |

## Chess Pieces
|        | Basic movement | Pin detection | Castling | En passant | Promotion |
|:------:|:--------------:|:-------------:|:--------:|:----------:|:---------:|
| King   |✓               |✓              |✓         |            |           |
| Pawn   |✓               |✓              |          |✓           |✓         |
| Knight |✓               |✓              |          |            |           |
| Bishop |✓               |✓              |          |            |           |
| Rook   |✓               |✓              |          |            |           |
| Queen  |✓               |✓              |          |            |           |

## Coding Style

|  Status  | Rule |
|:--------:|------|
|?         |1. [Every program must have a header.](https://hackmd.io/4X5HySkjTaSFdN6L_cnTwQ#1-Every-program-must-have-a-header-10%E5%88%86)|
|?         |2. [Use blank lines to separate logical sections.](https://hackmd.io/4X5HySkjTaSFdN6L_cnTwQ#2-Use-blank-lines-to-separate-logical-sections-5%E5%88%86)|
|✓         |3. [Use spaces around ‘=’ and around operators and after commas and semicolons.](https://hackmd.io/4X5HySkjTaSFdN6L_cnTwQ#3-Use-spaces-around-%E2%80%98%E2%80%99-and-around-operators-and-after-commas-and-semicolons-5%E5%88%86)|
|?         |4. [Use comments to describe major sections of program or where something needs to be clarified.](https://hackmd.io/4X5HySkjTaSFdN6L_cnTwQ#4-Use-comments-to-describe-major-sections-of-program-or-where-something-needs-to-be-clarified-10%E5%88%86)|
|✓         |5. [For names of objects (variables) you will use lower case letters and capitalize the first letter of the second and succeeding words.](https://hackmd.io/4X5HySkjTaSFdN6L_cnTwQ#5-For-names-of-objects-variables-you-will-use-lower-case-letters-and-capitalize-the-first-letter-of-the-second-and-succeeding-words-10%E5%88%86)|
|✓         |6. [For constants (including enumeration values), the identifier should be all capital letters (uppercase) using underscore to separate words.](https://hackmd.io/4X5HySkjTaSFdN6L_cnTwQ#6-For-constants-including-enumeration-values-the-identifier-should-be-all-capital-letters-uppercase-using-underscore-to-separate-words-10%E5%88%86)
|✓         |7. [The names of classes should start with an upper case letter.](https://hackmd.io/4X5HySkjTaSFdN6L_cnTwQ#7-The-names-of-classes-should-start-with-an-upper-case-letter-10%E5%88%86)|
|?         |8. [Use descriptive object and class names which relate the program to the problem.](https://hackmd.io/4X5HySkjTaSFdN6L_cnTwQ#8-Use-descriptive-object-and-class-names-which-relate-the-program-to-the-problem-10%E5%88%86)|
|✓         |9. [A class should be declared in a header file and defined in a source file where the name of the files match the name of the class.](https://hackmd.io/4X5HySkjTaSFdN6L_cnTwQ#9-A-class-should-be-declared-in-a-header-file-and-defined-in-a-source-file-where-the-name-of-the-files-match-the-name-of-the-class-5%E5%88%86)
|?         |10. [Indent if, for and do-while as shown.](https://hackmd.io/4X5HySkjTaSFdN6L_cnTwQ#10-Indent-if-for-and-do-while-as-shown-10%E5%88%86)|
|✓         |11. [All functions must have a series of comments which state the intent and the pre and post conditions. A pre-condition is a sentence or two which states what must be true before the function is called. The post-condition states what is true after the function is called.](https://hackmd.io/4X5HySkjTaSFdN6L_cnTwQ#11-All-functions-must-have-a-series-of-comments-which-state-the-intent-and-the-pre-and-post-conditions-A-pre-condition-is-a-sentence-or-two-which-states-what-must-be-true-before-the-function-is-called-The-post-condition-states-what-is-true-after-the-function-is-called-10%E5%88%86)|
|✓         |12. [Header files must contain an include guard.](https://hackmd.io/4X5HySkjTaSFdN6L_cnTwQ#12-Header-files-must-contain-an-include-guard-5%E5%88%86)|
