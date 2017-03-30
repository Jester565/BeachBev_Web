'use strict';
var resumeManager;
var innerLoginManager;

var BUCKET_REGION = 'us-west-1';
var BUCKET_NAME = 'beachbev-resumes'

var resumeManager = null;
var resumeZone = null;

var client = new Client(function (root) {
	console.log("ON LOAD CALLED");
	innerLoginManager = new InnerLoginManager(client, root,
		function () {
			console.log("LOGGED IN");
			resumeManager = new ResumeManager(client.root);
		});
	client.tcpConnection.onclose = function () {
		redirect('./noServer.html');
	};
});

function ResumeManager(root) {
	resumeManager = this;

	this.s3Client = null;
	this.s3Prefix = null;
	this.pdf = null;
	this.pdfIndex = 0;

	this.setErrorMsg = function (msg) {
		$('#msg').text(msg);
		$('#msg').removeClass('hidden');
		$('#msg').focus();
		$('html, body').scrollTo($('#msg'), 100);
	}

	this.clearErrorMsg = function () {
		$('#msg').addClass('hidden');
	}

	this.initDisplay = function () {
		$('#uploadDiv').removeClass('hidden');
		$('#loading').addClass('hidden');
	}

	this.initAWS = function (resumeCreds) {
		AWS.config.update({
			credentials: resumeCreds,
			region: BUCKET_REGION
		});
		resumeManager.s3Client = new AWS.S3({ apiVersion: '2006-03-01' });
	}

	this.initPackets = function (root) {
		resumeManager.PacketD0 = root.lookup("ProtobufPackets.PackD0");
		resumeManager.PacketD1 = root.lookup("ProtobufPackets.PackD1");
		client.packetManager.addPKey(new PKey("D1", function (iPack) {
			var packD1 = resumeManager.PacketD1.decode(iPack.packData);
			if (packD1.accessKey.length > 0) {
				resumeManager.s3Prefix = packD1.folderObjKey;
				var credentials = new AWS.Credentials(packD1.accessKeyID, packD1.accessKey, packD1.sessionKey);
				resumeManager.initAWS(credentials);
				resumeManager.loadResumes();
			}
			else {
				console.log(packD1.msg);
			}
		}));
	}

	this.sendD0 = function () {
		var packD0 = resumeManager.PacketD0.create({});
		client.tcpConnection.sendPack(new OPacket("D0", true, [0], packD0, resumeManager.PacketD0));
	}

	this.loadResumes = function () {
		var resumeFolderKey = encodeURIComponent(resumeManager.s3Prefix) + '/';
		var params = {
			Bucket: BUCKET_NAME,
			Prefix: resumeFolderKey,
			MaxKeys: 100,
			FetchOwner: false,
			EncodingType: 'url',
		}
		resumeManager.s3Client.listObjectsV2(params, function (err, data) {
			if (err) {
				resumeManager.setErrorMsg("Could not load resume folder: " + err.message);
			}
			else {
				var files = data.Contents.map(function (file) {
					var fileName = file.Key.substr(file.Key.indexOf('/') + 1);
					if (fileName.length > 0) {
						file.size = file.Size;
						file.name = fileName;
						file.type = 'application/pdf';
						file.uploaded = true;
						resumeZone.addFile(file);
						resumeZone.emit("complete", file);
					}
				});
			}
			resumeManager.initDisplay();
		})
	}

	this.uploadResume = function (file) {
		var fileKey = encodeURIComponent(resumeManager.s3Prefix) + '/' + file.name;
		resumeManager.s3Client.upload({
			Bucket: BUCKET_NAME,
			Key: fileKey,
			ContentType: file.type,
			Body: file
		}, function (err, data) {
			if (err) {
				resumeManager.setErrorMsg(err.message);
				$('#uploadButton h2').text('Failed');
			}
			else {
				file.uploaded = true;
				resumeZone.emit("complete", file);
				resumeZone.emit("success", file);
				$('#uploadButton h2').text('Complete');
			}
		});
	}

	this.viewResume = function (file) {
		if (file.uploaded) {
			var fileKey = encodeURIComponent(resumeManager.s3Prefix) + '/' + file.name;
			resumeManager.s3Client.getObject({
				Bucket: BUCKET_NAME,
				Key: fileKey
			}, function (err, data) {
				if (err) {
					console.log(err.message);
				}
				else
				{
					var fileArr = data.Body;
					resumeManager.showPDF(file.name, fileArr);
				}
			});
		}
		else
		{
			var fReader = new FileReader();
			fReader.onload = function () {
				var arr = new Uint8Array(this.result);
				resumeManager.showPDF(file.name, arr);
			}
			fReader.readAsArrayBuffer(file);
		}
	}

	this.showPDF = function (name, fileArr) {
		$('#pdfTitle').text(name);
		resumeManager.pdfIndex = 1;
		PDFJS.getDocument(fileArr).then(function (pdf) {
			resumeManager.pdf = pdf;
			resumeManager.loadPDFPage = function (page) {
				var scale = 1.5;
				var viewport = page.getViewport(scale);

				var canvas = document.createElement('canvas');
				canvas.style.display = 'block';
				var context = canvas.getContext('2d');
				canvas.height = viewport.height;
				canvas.width = viewport.width;
				canvas.className = 'pdfPage';

				var renderContext = {
					canvasContext: context,
					viewport: viewport
				};
				page.render(renderContext);

				$('#pdfDiv').append(canvas);
				$('#pdfDiv').css('width', String(canvas.width));
				$('#pdfHeadDiv').css('width', String(canvas.width));

				resumeManager.pdfIndex++;
				if (resumeManager.pdfIndex <= resumeManager.pdf.numPages) {
					pdf.getPage(resumeManager.pdfIndex).then(resumeManager.loadPDFPage);
				}
				else
				{
					$('body').css('background-color', 'lightgrey');
					$('#pdfBackDiv').removeClass('hidden');
					$('#pdfExit').click(function () {
						$('body').css('background-color', 'white');
						$('#pdfBackDiv').addClass('hidden');
						$('.pdfPage').remove();
						$('#pdfExit').unbind('click');
						resumeManager.pdf = null;
						resumeManager.pdfIndex = 0;
					});
				}
			}
			pdf.getPage(resumeManager.pdfIndex).then(resumeManager.loadPDFPage);
		});
	}
	this.initPackets(root);
	this.sendD0();
}

Dropzone.options.resumezone = {
	maxFilesize: 2,
	autoProcessQueue: false,
	maxFiles: 1,
	acceptedFiles: "application/pdf",
	thumbnailWidth: 150,
	thumbnailHeight: 190,
	clickable: true,
	url: function (file) {

	},
	init: function () {
		resumeZone = this;

		this.on("addedfile", function (file) {
			resumeManager.clearErrorMsg();
			file.previewElement.querySelector("img").src = "./images/pdfIcon.png";
			file.previewElement.addEventListener("click", function () {
				resumeManager.viewResume(file);
			});
			if (!file.uploaded) {
				$('#uploadButton').addClass('active');
				$('#uploadButton h2').text('Upload');
				$('#uploadButton').click(function () {
					if (resumeZone.files.length > 0) {
						if (!resumeZone.files[0].uploaded) {
							resumeManager.clearErrorMsg();
							$('#uploadButton').removeClass('active');
							$('#uploadButton h2').text('Uploading...');
							$('#uploadButton').unbind('click');
							resumeManager.uploadResume(resumeZone.files[0]);
						}
						else
						{
							resumeManager.setErrorMsg('File is already uploaded');
						}
					}
					else
					{
						resumeManager.setErrorMsg('No files to upload');
					}
				});
			}
		});

		this.on("error", function (file) {
			if (!file.accepted) {
				if (file.type !== 'application/pdf') {
					this.removeFile(file);
					resumeManager.setErrorMsg("File was not a pdf");
				}
			}
		});

		this.on("maxfilesexceeded", function (file) {
			this.removeAllFiles();
			this.addFile(file);
		});
	}
}