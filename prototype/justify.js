function justify(string)
{
  var lines = [];
  var line_length   = 40;
  var length_left   = function() { return (4 - lines.length) * line_length; }
  var current_line  = "";

  var words = string.split(/\s+/);
  while (words.length > 0)
  {
    current_word  = words.shift()

    is_first_word = current_line.length === 0
    next_length   = is_first_word ? " ".length : 0;
    next_length  += current_line.length + current_word.length;

    if (is_first_word && next_length > line_length)
    {
      // we’re the first word on this line, but we won’t fit, we must split it!
      first_slice  = current_word.substring(0, max_line_length);

      // next word will be our next slice
      second_slice = current_word.substring(max_line_length);
      words.unshift(second_slice);

      // include this slice for this line
      current_line += first_slice;
      lines.push(current_line);

      // we begin a new line!
      current_line  = "";
    }
    else if (next_length >= line_length)
    {
      // we’re not the first word, but the current word won’t fit; split it or move it to next line
      if ([current_word].concat(words).join(" ").length <= length_left())
      {
        // screw it, we move the word one line down
        words.unshift(current_word);

        // and begin a new line for next iteration
        lines.push(current_line);
        current_line = "";
      }
      else
      {
        // we must split this word :(
        cut_position = line_length - (current_line.length + " ".length);
        first_slice  = current_word.substring(0, cut_position);

        // put second slice as first word of next line
        second_slice = current_word.substring(cut_position);
        words.unshift(second_slice);

        // and include this slice in this line; and switch
        current_line += " " + first_slice;
        lines.push(current_line);

        // begin a new line
        current_line = "";
      }
    }
    else
    {
      // we’re safe to include this word no matter what we do
      if ( ! is_first_word)
      {
        current_line += " ";
      }

      current_line += current_word;
    }

  }

  if (current_line.length > 0)
  {
    lines.push(current_line);
  }

  return lines;
}
