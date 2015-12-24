#!/usr/bin/env node

'use strict';

const bigInt = require( 'big-integer' );
const fs     = require( 'fs' );
const util   = require( 'util' );

const sketch = fs.readFileSync( process.argv[2], 'utf8' );

const regexen = {
	beginSprite : /^sprite\s[^(]+\(\s*(\d+)\s*,\s*(\d+)\s*,\s*(0x[0-9a-f]+)\s*\)\s*;\s*\/\*\s*$/i,
	spriteData  : /(0|1)/g,
	endSprite   : /^\s*\*\/\s*$/
};

let inSprite = false;
let s = {};

sketch.split( '\n' ).map( line => line.trim() ).forEach( line => {
	if ( inSprite ) {
		// Currently in a sprite definition
		if ( line.match( regexen.endSprite ) ) {
			// This is the end of the sprite definition:
			// */
			inSprite = false;
			processSprite( s );
		} else {
			// This is a data line:
			//   1 0 1 0
			const bits = ( line.match( regexen.spriteData ) || [] ).map( b => +b );
			if ( bits.length !== s.width ) {
				throw new Error( util.format(
					'Invalid data line - expected %d bits but found %d (%s)',
					s.width,
					bits.length,
					line
				) );
			}
			s.pixels.push( bits );
		}
	} else {
		// Not in a sprite definition
		const match = line.match( regexen.beginSprite );
		if ( match ) {
			// This is the beginning of a new sprite definition:
			// sprite s_name(w, h, 0xdata); /*
			inSprite = true;
			s = {
				declaration : line.split( ';' )[0].trim(),
				width       : +match[1],
				height      : +match[2],
				data        : match[3],
				pixels      : []
			};
			if ( s.width >= 16 ) {
				throw new Error( 'Sprites can be at most 15px wide: ' + s.declaration );
			}
			if ( s.height >= 16 ) {
				throw new Error( 'Sprites can be at most 15px tall: ' + s.declaration );
			}
			if ( s.width * s.height > 64 ) {
				throw new Error( 'Sprites can have at most 64 pixels: ' + s.declaration );
			}
		}
	}
} );

function processSprite( s ) {
	console.log( s.declaration );
	if ( s.pixels.length !== s.height ) {
		throw new Error( util.format(
			'Invalid sprite - expected %d data lines but found %d',
			s.height,
			s.pixels.length
		) );
	}
	const pixels = [].concat.apply([], s.pixels);
	let data = bigInt( 0 );
	// for ( let i = 0; i < pixels.length; i++ ) {
	for ( let i = pixels.length - 1; i >= 0; i-- ) {
		data = data.shiftLeft( 1 ).or( pixels[i] );
	}
	data = Array( 16 + 1 ).join( '0' ) + data.toString( 16 );
	data = '0x' + data.substring( data.length - 16 );
	if ( data === s.data.toLowerCase() ) {
		console.log( '  -> Up to date' );
	} else {
		console.log( '  -> Needs update - data: %s', data );
	}
}
