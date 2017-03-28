'use strict';
var resumeManager;
var innerLoginManager;

var CRED_DELAY_MIN = 15;
var BUCKET_REGION = 'us-west-1';
var BUCKET_NAME = 'beachbev-resumes'

var resumeManager = null;
var resumeZone = null;

/*
var client = new Client(function (root) {
	console.log("ON LOAD CALLED");
	innerLoginManager = new InnerLoginManager(client, root,
		function () {
			console.log("LOGGED IN");
			resumeManager = new ResumeManager(client.root);
		});
	client.tcpConnection.onclose = function () {
		//redirect('./noServer.html');
		resumeManager = new ResumeManager(client.root);
	};
});
*/


resumeManager = new ResumeManager();

function ResumeManager(root) {
	resumeManager = this;

	this.s3Client = null;
	this.s3Prefix = null;
	this.s3URL = null;
	this.pdf = null;
	this.pdfIndex = 0;

	this.setErrorMsg = function (msg) {
		$('#msg').text(msg);
		$('#msg').removeClass('hidden');
		$('#msg').focus();
		$('html, body').scrollTo($('#msg'), 100);
	}

	this.initAWS = function (resumeCreds) {
		AWS.config.update({
			credentials: resumeCreds,
			region: BUCKET_REGION
		});

		resumeManager.s3Client = new AWS.S3({ apiVersion: '2006-03-01' });
	}

	this.initAWSNoPacks = function () {
		var credentials = new AWS.Credentials("AKIAISOB52YUY7N6C3OQ", "QVx6tG7WZtIidZse6Kcj9v1N+XzZNBXRGhS0+dOd");
		this.s3Prefix = '1';
		resumeManager.initAWS(credentials);
	}

	this.initPackets = function (root) {
		resumeManager.PacketD0 = root.lookup("ProtobufPackets.PackD0");
		resumeManager.PacketD1 = root.lookup("ProtobufPackets.PackD1");
		client.packetManager.addPKey(new PKey("D1", function (iPack) {
			var packD1 = resumeManager.PacketD1.decode(iPack.packData);
			var credentials = new AWS.Credentials();
			credentials.accessKeyID = packD1.accessKeyID;
			credentials.secretAccessKey = packD1.accessKey;
			credentials.sessionToken = packD1.sessionToken;
			credentials.expireTime = new Date(new Date().getTime() + CRED_DELAY_MIN * 600000);
			credentials.expired = false;
		}));
	}

	this.uploadResume = function (file) {
		var fileKey = encodeURIComponent(resumeManager.s3Prefix) + '/' + file.name;
		resumeManager.s3Client.upload({
			Bucket: 'beachbev-resumes',
			Key: fileKey,
			ContentType: file.type,
			Body: file
		}, function (err, data) {
			if (err) {
				console.log(err.message);
			}
			else {
				resumeManager.s3URL = this.request.httpRequest.endpoint.href + BUCKET_NAME + '/';
				file.uploaded = true;
				resumeZone.emit("complete", file);
				resumeZone.emit("success", file);
			}
		});
	}

	this.viewResume = function (file) {
		if (file.uploaded) {
			var fileKey = encodeURIComponent(resumeManager.s3Prefix) + '/' + file.name;
			resumeManager.s3Client.getObject({
				Bucket: 'beachbev-resumes',
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
					$('#pdfBackDiv').removeClass('hidden');
				}
			}
			pdf.getPage(resumeManager.pdfIndex).then(resumeManager.loadPDFPage);
		});
	}

	this.loadResumes = function (arn) {
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
				resumeManager.s3URL = this.request.httpRequest.endpoint.href + BUCKET_NAME + '/';
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
		})
	}

	this.initDisplay = function () {
		$('#uploadDiv').removeClass('hidden');
		$('#loading').addClass('hidden');
	}

	this.initAWSNoPacks();
	this.loadResumes();
	this.initDisplay();
}

Dropzone.options.resumezone = {
	maxFilesize: 2,
	autoProcessQueue: false,
	maxFiles: 1,
	acceptedFiles: "application/pdf",
	thumbnailWidth: 150,
	thumbnailHeight: 190,
	clickable: true,
	accept: function (file, done) {
		done();
	},
	url: function (file) {

	},
	init: function () {
		resumeZone = this;
		$('#uploadButton').click(function () {
			console.log(resumeZone.files.length);
			if (resumeZone.files.length > 0) {
				if (!resumeZone.files[0].uploaded) {
					resumeManager.uploadResume(resumeZone.files[0]);
				}
			}
		});

		this.on("addedfile", function (file) {
			file.previewElement.querySelector("img").src = "./images/pdfIcon.png";
			file.previewElement.addEventListener("click", function () {
				resumeManager.viewResume(file);
			});
		});

		this.on("maxfilesexceeded", function (file) {
			this.removeAllFiles();
			this.addFile(file);
		});
	}
}