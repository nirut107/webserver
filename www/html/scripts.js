async function testGet() {
	try {
		const path = document.getElementById('getPath').value;
		const response = await fetch('/' + path);
		const text = await response.text();
		document.getElementById('getResponse').innerHTML = `Response: ${text}`;
	} catch (error) {
		document.getElementById('getResponse').innerHTML = `Error: ${error.message}`;
	}
}

async function uploads() {
	try {
		const response = await fetch('/uploads');
		const text = await response.text();
		document.getElementById('uploadStore').innerHTML = text;
	} catch (error) {
		document.getElementById('uploadStore').innerHTML = `Error: ${error.message}`;
	}
}

document.getElementById('uploadForm').onsubmit = async (e) => {
	e.preventDefault();
	const formData = new FormData();
	const fileInput = document.getElementById('fileInput');
	formData.append('file', fileInput.files[0]);

	try {
		const response = await fetch('/uploads', {
			method: 'POST',
			body: formData
		});
		const result = await response.text();
		const response2 = await fetch('/uploads')
		const result2 = await response2.text();
		document.getElementById('uploadStore').innerHTML = result2;
		document.getElementById('uploadResponse').innerHTML = `Upload Result: ${result}`;
	} catch (error) {
		document.getElementById('uploadResponse').innerHTML = `Error: ${error.message}`;
	}
};

async function testDelete() {
	const filename = document.getElementById('deleteFile').value;
	if (!filename) {
		alert('Please enter a filename');
		return;
	}

	try {
		const response = await fetch(`/uploads/${filename}`, {
			method: 'DELETE'
		});
		const result = await response.text();
		const response2 = await fetch('/uploads')
		const result2 = await response2.text();
		document.getElementById('uploadStore').innerHTML = result2;
		document.getElementById('deleteResponse').innerHTML = `Delete Result: ${result}`;
	} catch (error) {
		document.getElementById('deleteResponse').innerHTML = `Error: ${error.message}`;
	}
}