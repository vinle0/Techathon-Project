// import { openai } from '@ai-sdk/openai';
//import { something } from 'https://esm.sh/@ai-sdk/openai';
//import { generateText } from 'ai';

// const { text } = await generateText({
//  model: openai('gpt-4o'),
//  prompt: 'Write a vegetarian lasagna recipe for 4 people.',
// });
// console.log("GenAI TEST:")
// console.log(text);

// import { generateText } from 'https://esm.sh/@ai-sdk/openai';
// console.log(generateText);

// import * as OpenAI from 'https://esm.sh/@ai-sdk/openai';
// console.log(OpenAI);


// function generateStudyMaterials() {
  
// }

// var content = "Here are some notes a user took while reading a book. First, determine whether the notes are from a fictional or nonfictional book. Then, based on that classification, generate appropriate study materials. For fiction, provide a summary, key themes, and 3 discussion questions. For nonfiction, provide a summary, key concepts, and 3 quiz questions.";


// const response = await fetch('http://127.0.0.1:5000/chat', {
//   method: 'POST',
//   headers: { 'Content-Type': 'application/json' },
//   body: JSON.stringify({ message: content })
// });

// const data = await response.json();
// console.log(data.reply);


// static/genAI.js

async function queryOpenAI(message) {
  try {
    const response = await fetch('http://127.0.0.1:5000/chat', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ message: message })
    });

    const data = await response.json();

    if (data.reply) {
      console.log('AI Reply:', data.reply);
      return data.reply;
    } else if (data.error) {
      console.error('API Error:', data.error);
      return `Error: ${data.error}`;
    }

  } catch (err) {
    console.error('Fetch Error:', err);
    return `Error: ${err.message}`;
  }
}


