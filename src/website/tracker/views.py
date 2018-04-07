from django.shortcuts import render

# Create your views here.

from django.http import HttpResponse

def index(request):
    return render(request, 'index.html', {})

import datetime
def get_time(request):
    time = datetime.datetime.now().time()
    return HttpResponse(time)
